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

void ComputeSystemDX12::Initialize(ID3D12Device *inDevice)
{
	mDevice = inDevice;

	// Dynamically load dxcompiler.dll
	HMODULE dxc_module = LoadLibraryW(L"dxcompiler.dll");
	mDxcCreateInstanceFn = dxc_module != nullptr? GetProcAddress(dxc_module, "DxcCreateInstance") : nullptr;
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
	String file_name = String(inName) + ".dxil";
	if (!mShaderLoader(file_name.c_str(), data, error))
	{
		result.SetError(error);
		return result;
	}

	// Create IDxcUtils object
	if (mDxcCreateInstanceFn == nullptr)
	{
		result.SetError("Failed to load dxcompiler.dll");
		return result;
	}
	ComPtr<IDxcUtils> utils;
	reinterpret_cast<DxcCreateInstanceProc>(reinterpret_cast<void *>(mDxcCreateInstanceFn))(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()));

	// Get reflection data
	DxcBuffer reflection_buffer = { data.data(), data.size(), 0 };
	ComPtr<ID3D12ShaderReflection> reflector;
	if (HRFailed(utils->CreateReflection(&reflection_buffer, IID_PPV_ARGS(reflector.GetAddressOf())), result))
		return result;

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
	compute_state_desc.CS = { data.data(), data.size() };
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

	result.Set(new ComputeShaderDX12(root_sig, pipeline_state, std::move(binding_names), std::move(name_to_index), inGroupSizeX, inGroupSizeY, inGroupSizeZ));
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
