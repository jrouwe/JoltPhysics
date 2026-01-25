// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_DX12

#include <Jolt/Compute/DX12/ComputeSystemDX12.h>
#include <Jolt/Compute/DX12/ComputeQueueDX12.h>
#include <Jolt/Compute/DX12/ComputeShaderDX12.h>
#include <Jolt/Compute/DX12/ComputeBufferDX12.h>
#include <Jolt/Core/StringTools.h>
#include <Jolt/Core/UnorderedMap.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
JPH_MSVC_SUPPRESS_WARNING(5204) // 'X': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
JPH_MSVC2026_PLUS_SUPPRESS_WARNING(4865) // wingdi.h(2806,1): '<unnamed-enum-DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER>': the underlying type will change from 'int' to '__int64' when '/Zc:enumTypes' is specified on the command line
#include <fstream>
#include <d3dcompiler.h>
#include <dxcapi.h>
#ifdef JPH_DEBUG
	#include <d3d12sdklayers.h>
#endif
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_RTTI_VIRTUAL(ComputeSystemDX12)
{
	JPH_ADD_BASE_CLASS(ComputeSystemDX12, ComputeSystem)
}

void ComputeSystemDX12::Initialize(ID3D12Device *inDevice, EDebug inDebug)
{
	mDevice = inDevice;
	mDebug = inDebug;
}

void ComputeSystemDX12::Shutdown()
{
	mDevice.Reset();
}

ComPtr<ID3D12Resource> ComputeSystemDX12::CreateD3DResource(D3D12_HEAP_TYPE inHeapType, D3D12_RESOURCE_STATES inResourceState, D3D12_RESOURCE_FLAGS inFlags, uint64 inSize)
{
	// Create a new resource
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = inSize;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = inFlags;

	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = inHeapType;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	ComPtr<ID3D12Resource> resource;
	if (HRFailed(mDevice->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, inResourceState, nullptr, IID_PPV_ARGS(&resource))))
		return nullptr;
	return resource;
}

ComputeShaderResult ComputeSystemDX12::CreateComputeShader(const char *inName, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ)
{
	ComputeShaderResult result;

	// Read shader source file
	Array<uint8> data;
	String error;
	String file_name = String(inName) + ".hlsl";
	if (!mShaderLoader(file_name.c_str(), data, error))
	{
		result.SetError(error);
		return result;
	}

#ifndef JPH_USE_DXC // Use FXC, the old shader compiler?

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND;
#ifdef JPH_DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	if (mDebug == EDebug::DebugSymbols)
		flags |= D3DCOMPILE_DEBUG;

	const D3D_SHADER_MACRO defines[] =
	{
		{ nullptr, nullptr }
	};

	// Handles loading include files through the shader loader
	struct IncludeHandler : public ID3DInclude
	{
								IncludeHandler(const ShaderLoader &inShaderLoader) : mShaderLoader(inShaderLoader) { }
		virtual					~IncludeHandler() = default;

		STDMETHOD				(Open)(D3D_INCLUDE_TYPE, LPCSTR inFileName, LPCVOID, LPCVOID *outData, UINT *outNumBytes) override
		{
			// Read the header file
			Array<uint8> file_data;
			String error;
			if (!mShaderLoader(inFileName, file_data, error))
				return E_FAIL;
			if (file_data.empty())
			{
				*outData = nullptr;
				*outNumBytes = 0;
				return S_OK;
			}

			// Copy to a new memory block
			void *mem = CoTaskMemAlloc(file_data.size());
			if (mem == nullptr)
				return E_OUTOFMEMORY;
			memcpy(mem, file_data.data(), file_data.size());
			*outData = mem;
			*outNumBytes = (UINT)file_data.size();
			return S_OK;
		}

		STDMETHOD				(Close)(LPCVOID inData) override
		{
			if (inData != nullptr)
				CoTaskMemFree(const_cast<void *>(inData));
			return S_OK;
		}

	private:
		const ShaderLoader &	mShaderLoader;
	};
	IncludeHandler include_handler(mShaderLoader);

	// Compile source
	ComPtr<ID3DBlob> shader_blob, error_blob;
	if (FAILED(D3DCompile(&data[0],
				(uint)data.size(),
				file_name.c_str(),
				defines,
				&include_handler,
				"main",
				"cs_5_0",
				flags,
				0,
				shader_blob.GetAddressOf(),
				error_blob.GetAddressOf())))
	{
		if (error_blob)
			result.SetError((const char *)error_blob->GetBufferPointer());
		else
			result.SetError("Shader compile error");
		return result;
	}

	// Get shader description
	ComPtr<ID3D12ShaderReflection> reflector;
	if (FAILED(D3DReflect(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), IID_PPV_ARGS(&reflector))))
	{
		result.SetError("Failed to reflect shader");
		return result;
	}

#else

	ComPtr<IDxcUtils> utils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()));

	// Custom include handler that forwards include loads to mShaderLoader
	struct DxcIncludeHandler : public IDxcIncludeHandler
	{
								DxcIncludeHandler(IDxcUtils *inUtils, const ShaderLoader &inLoader) : mUtils(inUtils), mShaderLoader(inLoader) { }
		virtual					~DxcIncludeHandler() = default;

		STDMETHODIMP			QueryInterface(REFIID riid, void **ppvObject) override
		{
			JPH_ASSERT(false);
			return E_NOINTERFACE;
		}

		STDMETHODIMP_(ULONG)	AddRef(void) override
		{
			// Allocated on the stack, we don't do ref counting
			return 1;
		}

		STDMETHODIMP_(ULONG)	Release(void) override
		{
			// Allocated on the stack, we don't do ref counting
			return 1;
		}

		// IDxcIncludeHandler::LoadSource uses IDxcBlob**
		STDMETHODIMP			LoadSource(LPCWSTR inFilename, IDxcBlob **outIncludeSource) override
		{
			*outIncludeSource = nullptr;

			// Convert to UTF-8
			char file_name[MAX_PATH];
			WideCharToMultiByte(CP_UTF8, 0, inFilename, -1, file_name, sizeof(file_name), nullptr, nullptr);

			// Load the header
			Array<uint8> file_data;
			String error;
			if (!mShaderLoader(file_name, file_data, error))
				return E_FAIL;

			// Create a blob from the loaded data
			ComPtr<IDxcBlobEncoding> blob_encoder;
			HRESULT hr = mUtils->CreateBlob(file_data.empty()? nullptr : file_data.data(), (uint)file_data.size(), CP_UTF8, blob_encoder.GetAddressOf());
			if (FAILED(hr))
				return hr;

			// Return as IDxcBlob
			*outIncludeSource = blob_encoder.Detach();
			return S_OK;
		}

		IDxcUtils *				mUtils;
		const ShaderLoader &	mShaderLoader;
	};
	DxcIncludeHandler include_handler(utils.Get(), mShaderLoader);

	ComPtr<IDxcBlobEncoding> source;
	if (HRFailed(utils->CreateBlob(data.data(), (uint)data.size(), CP_UTF8, source.GetAddressOf()), result))
		return result;

	ComPtr<IDxcCompiler3> compiler;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.GetAddressOf()));

	Array<LPCWSTR> arguments;
	arguments.push_back(L"-E");
	arguments.push_back(L"main");
	arguments.push_back(L"-T");
	arguments.push_back(L"cs_6_0");
	arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
	arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
	arguments.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
	if (mDebug == EDebug::DebugSymbols)
	{
		arguments.push_back(DXC_ARG_DEBUG);
		arguments.push_back(L"-Qembed_debug");
	}

	// Provide file name so tools know what the original shader was called (the actual source comes from the blob)
	wchar_t w_file_name[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, file_name.c_str(), -1, w_file_name, MAX_PATH);
	arguments.push_back(w_file_name);

	// Compile the shader
	DxcBuffer source_buffer;
	source_buffer.Ptr = source->GetBufferPointer();
	source_buffer.Size = source->GetBufferSize();
	source_buffer.Encoding = 0;
	ComPtr<IDxcResult> compile_result;
	if (FAILED(compiler->Compile(&source_buffer, arguments.data(), (uint32)arguments.size(), &include_handler, IID_PPV_ARGS(compile_result.GetAddressOf()))))
	{
		result.SetError("Failed to compile shader");
		return result;
	}

	// Check for compilation errors
	ComPtr<IDxcBlobUtf8> errors;
	compile_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);
	if (errors != nullptr && errors->GetStringLength() > 0)
	{
		result.SetError((const char *)errors->GetBufferPointer());
		return result;
	}

	// Get the compiled shader code
	ComPtr<ID3DBlob> shader_blob;
	if (HRFailed(compile_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(shader_blob.GetAddressOf()), nullptr), result))
		return result;

	// Get reflection data
	ComPtr<IDxcBlob> reflection_data;
	if (HRFailed(compile_result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflection_data.GetAddressOf()), nullptr), result))
		return result;
	DxcBuffer reflection_buffer;
	reflection_buffer.Ptr = reflection_data->GetBufferPointer();
	reflection_buffer.Size = reflection_data->GetBufferSize();
	reflection_buffer.Encoding = 0;
	ComPtr<ID3D12ShaderReflection> reflector;
	if (HRFailed(utils->CreateReflection(&reflection_buffer, IID_PPV_ARGS(reflector.GetAddressOf())), result))
		return result;

#endif // JPH_USE_DXC

	// Get the shader description
	D3D12_SHADER_DESC shader_desc;
	if (HRFailed(reflector->GetDesc(&shader_desc), result))
		return result;

	// Verify that the group sizes match the shader's thread group size
	UINT thread_group_size_x, thread_group_size_y, thread_group_size_z;
	if (HRFailed(reflector->GetThreadGroupSize(&thread_group_size_x, &thread_group_size_y, &thread_group_size_z), result))
		return result;
	JPH_ASSERT(inGroupSizeX == thread_group_size_x, "Group size X mismatch");
	JPH_ASSERT(inGroupSizeY == thread_group_size_y, "Group size Y mismatch");
	JPH_ASSERT(inGroupSizeZ == thread_group_size_z, "Group size Z mismatch");

	// Convert parameters to root signature description
	Array<String> binding_names;
	binding_names.reserve(shader_desc.BoundResources);
	UnorderedMap<string_view, uint> name_to_index;
	Array<D3D12_ROOT_PARAMETER1> root_params;
	for (UINT i = 0; i < shader_desc.BoundResources; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC bind_desc;
		reflector->GetResourceBindingDesc(i, &bind_desc);

		D3D12_ROOT_PARAMETER1 param = {};
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		switch (bind_desc.Type)
		{
		case D3D_SIT_CBUFFER:
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			break;

		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			break;

		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_UAV_RWSTRUCTURED:
		case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
			break;

        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
        case D3D_SIT_SAMPLER:
        case D3D_SIT_RTACCELERATIONSTRUCTURE:
		case D3D_SIT_UAV_FEEDBACKTEXTURE:
			JPH_ASSERT(false, "Unsupported shader input type");
			continue;
		}

		param.Descriptor.RegisterSpace = bind_desc.Space;
		param.Descriptor.ShaderRegister = bind_desc.BindPoint;
		param.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;

		binding_names.push_back(bind_desc.Name); // Add all strings to a pool to keep them alive
		name_to_index[string_view(binding_names.back())] = (uint)root_params.size();
		root_params.push_back(param);
	}

	// Create the root signature
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC root_sig_desc = {};
	root_sig_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	root_sig_desc.Desc_1_1.NumParameters = (UINT)root_params.size();
	root_sig_desc.Desc_1_1.pParameters = root_params.data();
	root_sig_desc.Desc_1_1.NumStaticSamplers = 0;
	root_sig_desc.Desc_1_1.pStaticSamplers = nullptr;
	root_sig_desc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	ComPtr<ID3DBlob> serialized_sig;
	ComPtr<ID3DBlob> root_sig_error_blob;
	if (FAILED(D3D12SerializeVersionedRootSignature(&root_sig_desc, &serialized_sig, &root_sig_error_blob)))
	{
		if (root_sig_error_blob)
		{
			error = StringFormat("Failed to create root signature: %s", (const char *)root_sig_error_blob->GetBufferPointer());
			result.SetError(error);
		}
		else
			result.SetError("Failed to create root signature");
		return result;
	}
	ComPtr<ID3D12RootSignature> root_sig;
	if (FAILED(mDevice->CreateRootSignature(0, serialized_sig->GetBufferPointer(), serialized_sig->GetBufferSize(), IID_PPV_ARGS(&root_sig))))
	{
		result.SetError("Failed to create root signature");
		return result;
	}

	// Create a pipeline state object from the root signature and the shader
	ComPtr<ID3D12PipelineState> pipeline_state;
	D3D12_COMPUTE_PIPELINE_STATE_DESC compute_state_desc = {};
	compute_state_desc.pRootSignature = root_sig.Get();
	compute_state_desc.CS = { shader_blob->GetBufferPointer(), shader_blob->GetBufferSize() };
	if (FAILED(mDevice->CreateComputePipelineState(&compute_state_desc, IID_PPV_ARGS(&pipeline_state))))
	{
		result.SetError("Failed to create compute pipeline state");
		return result;
	}

	// Set name on DX12 objects for easier debugging
	wchar_t w_name[1024];
	size_t converted_chars = 0;
	mbstowcs_s(&converted_chars, w_name, 1024, inName, _TRUNCATE);
	pipeline_state->SetName(w_name);

	result.Set(new ComputeShaderDX12(shader_blob, root_sig, pipeline_state, std::move(binding_names), std::move(name_to_index), inGroupSizeX, inGroupSizeY, inGroupSizeZ));
	return result;
}

ComputeBufferResult ComputeSystemDX12::CreateComputeBuffer(ComputeBuffer::EType inType, uint64 inSize, uint inStride, const void *inData)
{
	ComputeBufferResult result;

	Ref<ComputeBufferDX12> buffer = new ComputeBufferDX12(this, inType, inSize, inStride);
	if (!buffer->Initialize(inData))
	{
		result.SetError("Failed to create compute buffer");
		return result;
	}

	result.Set(buffer.GetPtr());
	return result;
}

ComputeQueueResult ComputeSystemDX12::CreateComputeQueue()
{
	ComputeQueueResult result;

	Ref<ComputeQueueDX12> queue = new ComputeQueueDX12();
	if (!queue->Initialize(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, result))
		return result;

	result.Set(queue.GetPtr());
	return result;
}

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
