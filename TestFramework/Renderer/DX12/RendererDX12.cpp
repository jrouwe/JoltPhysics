// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/DX12/RendererDX12.h>
#include <Renderer/DX12/RenderPrimitiveDX12.h>
#include <Renderer/DX12/PipelineStateDX12.h>
#include <Renderer/DX12/VertexShaderDX12.h>
#include <Renderer/DX12/PixelShaderDX12.h>
#include <Renderer/DX12/TextureDX12.h>
#include <Renderer/DX12/RenderInstancesDX12.h>
#include <Renderer/DX12/FatalErrorIfFailedDX12.h>
#include <Window/ApplicationWindowWin.h>
#include <Jolt/Core/Profiler.h>
#include <Utils/ReadData.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>

#include <d3dcompiler.h>
#ifdef JPH_DEBUG
	#include <d3d12sdklayers.h>
#endif

RendererDX12::~RendererDX12()
{
	// Ensure that the GPU is no longer referencing resources that are about to be cleaned up by the destructor.
	WaitForGpu();

	// Don't add more stuff to the delay reference list
	mIsExiting = true;

	CloseHandle(mFenceEvent);
}

void RendererDX12::WaitForGpu()
{
	// Schedule a Signal command in the queue
	UINT64 current_fence_value = mFenceValues[mFrameIndex];
	FatalErrorIfFailed(mCommandQueue->Signal(mFence.Get(), current_fence_value));

	// Wait until the fence has been processed
	FatalErrorIfFailed(mFence->SetEventOnCompletion(current_fence_value, mFenceEvent));
	WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);

	// Increment the fence value for all frames
	for (uint n = 0; n < cFrameCount; ++n)
		mFenceValues[n] = current_fence_value + 1;

	// Release all used resources
	for (Array<ComPtr<ID3D12Object>> &list : mDelayReleased)
		list.clear();

	// Anything that's not used yet can be removed, delayed objects are now available
	mResourceCache.clear();
	mDelayCached[mFrameIndex].swap(mResourceCache);
}

void RendererDX12::CreateRenderTargets()
{
	// Create render targets and views
	for (uint n = 0; n < cFrameCount; ++n)
	{
		mRenderTargetViews[n] = mRTVHeap.Allocate();

		FatalErrorIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));
		mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, mRenderTargetViews[n]);
	}
}

void RendererDX12::CreateDepthBuffer()
{
	// Free any previous depth stencil view
	if (mDepthStencilView.ptr != 0)
		mDSVHeap.Free(mDepthStencilView);

	// Free any previous depth stencil buffer
	mDepthStencilBuffer.Reset();

	// Allocate depth stencil buffer
	D3D12_CLEAR_VALUE clear_value = {};
	clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	clear_value.DepthStencil.Depth = 0.0f;
	clear_value.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC depth_stencil_desc = {};
	depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_desc.Alignment = 0;
	depth_stencil_desc.Width = mWindow->GetWindowWidth();
	depth_stencil_desc.Height = mWindow->GetWindowHeight();
	depth_stencil_desc.DepthOrArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_desc.SampleDesc.Count = 1;
	depth_stencil_desc.SampleDesc.Quality = 0;
	depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	FatalErrorIfFailed(mDevice->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &depth_stencil_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&mDepthStencilBuffer)));

	// Allocate depth stencil view
	D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
	depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;

	mDepthStencilView = mDSVHeap.Allocate();
	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &depth_stencil_view_desc, mDepthStencilView);
}

void RendererDX12::Initialize(ApplicationWindow *inWindow)
{
	Renderer::Initialize(inWindow);

#if defined(JPH_DEBUG)
	// Enable the D3D12 debug layer
	ComPtr<ID3D12Debug> debug_controller;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
		debug_controller->EnableDebugLayer();
#endif

	// Create DXGI factory
	FatalErrorIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mDXGIFactory)));

	// Find adapter
	ComPtr<IDXGIAdapter1> adapter;

	HRESULT result = E_FAIL;

	// First check if we have the Windows 1803 IDXGIFactory6 interface
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(mDXGIFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (UINT index = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)); ++index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			// We don't want software renderers
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			// Check to see whether the adapter supports Direct3D 12
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
			if (SUCCEEDED(result))
				break;
		}
	}
	else
	{
		// Fall back to the older method that may not get the fastest GPU
		for (UINT index = 0; DXGI_ERROR_NOT_FOUND != mDXGIFactory->EnumAdapters1(index, &adapter); ++index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			// We don't want software renderers
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			// Check to see whether the adapter supports Direct3D 12
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
			if (SUCCEEDED(result))
				break;
		}
	}

	// Check if we managed to obtain a device
	FatalErrorIfFailed(result);

#ifdef JPH_DEBUG
	// Enable breaking on errors
	ComPtr<ID3D12InfoQueue> info_queue;
	if (SUCCEEDED(mDevice.As(&info_queue)))
	{
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Disable an error that triggers on Windows 11 with a hybrid graphic system
		// See: https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
		D3D12_MESSAGE_ID hide[] =
		{
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
		};
		D3D12_INFO_QUEUE_FILTER filter = { };
		filter.DenyList.NumIDs = static_cast<UINT>( std::size( hide ) );
		filter.DenyList.pIDList = hide;
		info_queue->AddStorageFilterEntries( &filter );
	}
#endif // JPH_DEBUG

	// Disable full screen transitions
	FatalErrorIfFailed(mDXGIFactory->MakeWindowAssociation(static_cast<ApplicationWindowWin *>(mWindow)->GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));

	// Create heaps
	mRTVHeap.Init(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 2);
	mDSVHeap.Init(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 4);
	mSRVHeap.Init(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 128);

	// Create a command queue
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	FatalErrorIfFailed(mDevice->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&mCommandQueue)));

	// Create a command allocator for each frame
	for (uint n = 0; n < cFrameCount; n++)
		FatalErrorIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[n])));

	// Describe and create the swap chain
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
	swap_chain_desc.BufferCount = cFrameCount;
	swap_chain_desc.BufferDesc.Width = mWindow->GetWindowWidth();
	swap_chain_desc.BufferDesc.Height = mWindow->GetWindowHeight();
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.OutputWindow = static_cast<ApplicationWindowWin *>(mWindow)->GetWindowHandle();
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.Windowed = TRUE;

	ComPtr<IDXGISwapChain> swap_chain;
	FatalErrorIfFailed(mDXGIFactory->CreateSwapChain(mCommandQueue.Get(), &swap_chain_desc, &swap_chain));
	FatalErrorIfFailed(swap_chain.As(&mSwapChain));
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargets();

	CreateDepthBuffer();

	// Create a root signature suitable for all our shaders
	D3D12_ROOT_PARAMETER params[3] = {};

	// Mapping a constant buffer to slot 0 for the vertex shader
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.ShaderRegister = 0;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// Mapping a constant buffer to slot 1 in the pixel shader
	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.ShaderRegister = 1;
	params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Mapping a texture to slot 2 in the pixel shader
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.BaseShaderRegister = 2;
	range.NumDescriptors = 1;
	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &range;
	params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC samplers[3] = {};

	// Sampler 0: Non-wrapping linear filtering
	samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplers[0].MipLODBias = 0.0f;
	samplers[0].MaxAnisotropy = 1;
	samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplers[0].MinLOD = 0.0f;
	samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplers[0].ShaderRegister = 0;
	samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Sampler 1: Wrapping and linear filtering
	samplers[1] = samplers[0];
	samplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplers[1].ShaderRegister = 1;

	// Sampler 2: Point filtering, using SampleCmp mode to compare if sampled value >= reference value (for shadows)
	samplers[2] = samplers[0];
	samplers[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplers[2].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	samplers[2].ShaderRegister = 2;

	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	root_signature_desc.NumParameters = ARRAYSIZE(params);
	root_signature_desc.pParameters = params;
	root_signature_desc.NumStaticSamplers = ARRAYSIZE(samplers);
	root_signature_desc.pStaticSamplers = samplers;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	FatalErrorIfFailed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	FatalErrorIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

	// Create the command list
	FatalErrorIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[mFrameIndex].Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

	// Command lists are created in the recording state, but there is nothing to record yet. The main loop expects it to be closed, so close it now
	FatalErrorIfFailed(mCommandList->Close());

	// Create synchronization object
	FatalErrorIfFailed(mDevice->CreateFence(mFenceValues[mFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	// Increment fence value so we don't skip waiting the first time a command list is executed
	mFenceValues[mFrameIndex]++;

	// Create an event handle to use for frame synchronization
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
		FatalErrorIfFailed(HRESULT_FROM_WIN32(GetLastError()));

	// Initialize the queue used to upload resources to the GPU
	mUploadQueue.Initialize(mDevice.Get());

	// Create constant buffer. One per frame to avoid overwriting the constant buffer while the GPU is still using it.
	for (uint n = 0; n < cFrameCount; ++n)
	{
		mVertexShaderConstantBufferProjection[n] = CreateConstantBuffer(sizeof(VertexShaderConstantBuffer));
		mVertexShaderConstantBufferOrtho[n] = CreateConstantBuffer(sizeof(VertexShaderConstantBuffer));
		mPixelShaderConstantBuffer[n] = CreateConstantBuffer(sizeof(PixelShaderConstantBuffer));
	}

	// Create depth only texture (no color buffer, as seen from light)
	mShadowMap = new TextureDX12(this, cShadowMapSize, cShadowMapSize);
}

void RendererDX12::OnWindowResize()
{
	// Wait for the previous frame to be rendered
	WaitForGpu();

	// Free the render targets and views to allow resizing the swap chain
	for (uint n = 0; n < cFrameCount; ++n)
	{
		mRTVHeap.Free(mRenderTargetViews[n]);
		mRenderTargets[n].Reset();
	}

	// Resize the swap chain buffers
	FatalErrorIfFailed(mSwapChain->ResizeBuffers(cFrameCount, mWindow->GetWindowWidth(), mWindow->GetWindowHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Back buffer index may have changed after the resize (it always seems to go to 0 again)
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	// Since we may have switched frame index and we know everything is done, we need to update the fence value for our other frame as completed
	for (uint n = 0; n < cFrameCount; ++n)
		if (mFrameIndex != n)
			mFenceValues[n] = mFence->GetCompletedValue();

	// Recreate render targets
	CreateRenderTargets();

	// Recreate depth buffer
	CreateDepthBuffer();
}

bool RendererDX12::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	Renderer::BeginFrame(inCamera, inWorldScale);

	// Reset command allocator
	FatalErrorIfFailed(mCommandAllocators[mFrameIndex]->Reset());

	// Reset command list
	FatalErrorIfFailed(mCommandList->Reset(mCommandAllocators[mFrameIndex].Get(), nullptr));

	// Set root signature
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	// Set SRV heap
	ID3D12DescriptorHeap *heaps[] = { mSRVHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mRenderTargets[mFrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &barrier);

	// Clear the back buffer.
	const float blue[] = { 0.098f, 0.098f, 0.439f, 1.000f };
	mCommandList->ClearRenderTargetView(mRenderTargetViews[mFrameIndex], blue, 0, nullptr);
	mCommandList->ClearDepthStencilView(mDepthStencilView, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

	// Set constants for vertex shader in projection mode
	VertexShaderConstantBuffer *vs = mVertexShaderConstantBufferProjection[mFrameIndex]->Map<VertexShaderConstantBuffer>();
	*vs = mVSBuffer;
	mVertexShaderConstantBufferProjection[mFrameIndex]->Unmap();

	// Set constants for vertex shader in ortho mode
	vs = mVertexShaderConstantBufferOrtho[mFrameIndex]->Map<VertexShaderConstantBuffer>();
	*vs = mVSBufferOrtho;
	mVertexShaderConstantBufferOrtho[mFrameIndex]->Unmap();

	// Switch to 3d projection mode
	SetProjectionMode();

	// Set constants for pixel shader
	PixelShaderConstantBuffer *ps = mPixelShaderConstantBuffer[mFrameIndex]->Map<PixelShaderConstantBuffer>();
	*ps = mPSBuffer;
	mPixelShaderConstantBuffer[mFrameIndex]->Unmap();

	// Set the pixel shader constant buffer data.
	mPixelShaderConstantBuffer[mFrameIndex]->Bind(1);

	// Start drawing the shadow pass
	mShadowMap->SetAsRenderTarget(true);

	return true;
}

void RendererDX12::EndShadowPass()
{
	JPH_PROFILE_FUNCTION();

	// Finish drawing the shadow pass
	mShadowMap->SetAsRenderTarget(false);

	// Set the main back buffer as render target
	mCommandList->OMSetRenderTargets(1, &mRenderTargetViews[mFrameIndex], FALSE, &mDepthStencilView);

	// Set viewport
	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(mWindow->GetWindowWidth()), static_cast<float>(mWindow->GetWindowHeight()), 0.0f, 1.0f };
	mCommandList->RSSetViewports(1, &viewport);

	// Set scissor rect
	D3D12_RECT scissor_rect = { 0, 0, static_cast<LONG>(mWindow->GetWindowWidth()), static_cast<LONG>(mWindow->GetWindowHeight()) };
	mCommandList->RSSetScissorRects(1, &scissor_rect);
}

void RendererDX12::EndFrame()
{
	JPH_PROFILE_FUNCTION();

	Renderer::EndFrame();

	// Indicate that the back buffer will now be used to present.
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = mRenderTargets[mFrameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	mCommandList->ResourceBarrier(1, &barrier);

	// Close the command list
	FatalErrorIfFailed(mCommandList->Close());

	// Execute the command list
	ID3D12CommandList* command_lists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(command_lists), command_lists);

	// Present the frame
	FatalErrorIfFailed(mSwapChain->Present(1, 0));

	// Schedule a Signal command in the queue
	UINT64 current_fence_value = mFenceValues[mFrameIndex];
	FatalErrorIfFailed(mCommandQueue->Signal(mFence.Get(), current_fence_value));

	// Update the frame index
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready
	UINT64 completed_value = mFence->GetCompletedValue();
	if (completed_value < mFenceValues[mFrameIndex])
	{
		FatalErrorIfFailed(mFence->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent));
		WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
	}

	// Release all used resources
	mDelayReleased[mFrameIndex].clear();

	// Anything that's not used yet can be removed, delayed objects are now available
	mResourceCache.clear();
	mDelayCached[mFrameIndex].swap(mResourceCache);

	// Set the fence value for the next frame.
	mFenceValues[mFrameIndex] = current_fence_value + 1;
}


void RendererDX12::SetProjectionMode()
{
	JPH_ASSERT(mInFrame);

	mVertexShaderConstantBufferProjection[mFrameIndex]->Bind(0);
}

void RendererDX12::SetOrthoMode()
{
	JPH_ASSERT(mInFrame);

	mVertexShaderConstantBufferOrtho[mFrameIndex]->Bind(0);
}

Ref<Texture> RendererDX12::CreateTexture(const Surface *inSurface)
{
	return new TextureDX12(this, inSurface);
}

Ref<VertexShader> RendererDX12::CreateVertexShader(const char *inName)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef JPH_DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		{ nullptr, nullptr }
	};

	// Read shader source file
	String file_name = String("Shaders/DX/") + inName + ".hlsl";
	Array<uint8> data = ReadData(file_name.c_str());

	// Compile source
	ComPtr<ID3DBlob> shader_blob, error_blob;
	HRESULT hr = D3DCompile(&data[0],
							(uint)data.size(),
							(AssetStream::sGetAssetsBasePath() + file_name).c_str(),
							defines,
							D3D_COMPILE_STANDARD_FILE_INCLUDE,
							"main",
							"vs_5_0",
							flags,
							0,
							shader_blob.GetAddressOf(),
							error_blob.GetAddressOf());
	if (FAILED(hr))
	{
		// Throw error if compilation failed
		if (error_blob)
			OutputDebugStringA((const char *)error_blob->GetBufferPointer());
		FatalError("Failed to compile vertex shader");
	}

	return new VertexShaderDX12(shader_blob);
}

Ref<PixelShader> RendererDX12::CreatePixelShader(const char *inName)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef JPH_DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		{ nullptr, nullptr }
	};

	// Read shader source file
	String file_name = String("Shaders/DX/") + inName + ".hlsl";
	Array<uint8> data = ReadData(file_name.c_str());

	// Compile source
	ComPtr<ID3DBlob> shader_blob, error_blob;
	HRESULT hr = D3DCompile(&data[0],
							(uint)data.size(),
							(AssetStream::sGetAssetsBasePath() + file_name).c_str(),
							defines,
							D3D_COMPILE_STANDARD_FILE_INCLUDE,
							"main",
							"ps_5_0",
							flags,
							0,
							shader_blob.GetAddressOf(),
							error_blob.GetAddressOf());
	if (FAILED(hr))
	{
		// Throw error if compilation failed
		if (error_blob)
			OutputDebugStringA((const char *)error_blob->GetBufferPointer());
		FatalError("Failed to compile pixel shader");
	}

	return new PixelShaderDX12(shader_blob);
}

unique_ptr<ConstantBufferDX12> RendererDX12::CreateConstantBuffer(uint inBufferSize)
{
	return make_unique<ConstantBufferDX12>(this, inBufferSize);
}

unique_ptr<PipelineState> RendererDX12::CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode)
{
	return make_unique<PipelineStateDX12>(this, static_cast<const VertexShaderDX12 *>(inVertexShader), inInputDescription, inInputDescriptionCount, static_cast<const PixelShaderDX12 *>(inPixelShader), inDrawPass, inFillMode, inTopology, inDepthTest, inBlendMode, inCullMode);
}

RenderPrimitive *RendererDX12::CreateRenderPrimitive(PipelineState::ETopology inType)
{
	return new RenderPrimitiveDX12(this, inType);
}

RenderInstances *RendererDX12::CreateRenderInstances()
{
	return new RenderInstancesDX12(this);
}

ComPtr<ID3D12Resource> RendererDX12::CreateD3DResource(D3D12_HEAP_TYPE inHeapType, D3D12_RESOURCE_STATES inResourceState, uint64 inSize)
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
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heap_properties = {};
	heap_properties.Type = inHeapType;
	heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heap_properties.CreationNodeMask = 1;
	heap_properties.VisibleNodeMask = 1;

	ComPtr<ID3D12Resource> resource;
	FatalErrorIfFailed(mDevice->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, inResourceState, nullptr, IID_PPV_ARGS(&resource)));
	return resource;
}

void RendererDX12::CopyD3DResource(ID3D12Resource *inDest, const void *inSrc, uint64 inSize)
{
	// Copy data to destination buffer
	void *data;
	D3D12_RANGE range = { 0, 0 }; // We're not going to read
	FatalErrorIfFailed(inDest->Map(0, &range, &data));
	memcpy(data, inSrc, size_t(inSize));
	inDest->Unmap(0, nullptr);
}

void RendererDX12::CopyD3DResource(ID3D12Resource *inDest, ID3D12Resource *inSrc, uint64 inSize)
{
	// Start a commandlist for the upload
	ID3D12GraphicsCommandList *list = mUploadQueue.Start();

	// Copy the data to the GPU
	list->CopyBufferRegion(inDest, 0, inSrc, 0, inSize);

	// Change the state of the resource to generic read
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = inDest;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	list->ResourceBarrier(1, &barrier);

	// Wait for copying to finish
	mUploadQueue.ExecuteAndWait();
}

ComPtr<ID3D12Resource> RendererDX12::CreateD3DResourceOnDefaultHeap(const void *inData, uint64 inSize)
{
	ComPtr<ID3D12Resource> upload = CreateD3DResourceOnUploadHeap(inSize);
	ComPtr<ID3D12Resource> resource = CreateD3DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, inSize);
	CopyD3DResource(upload.Get(), inData, inSize);
	CopyD3DResource(resource.Get(), upload.Get(), inSize);
	RecycleD3DResourceOnUploadHeap(upload.Get(), inSize);
	return resource;
}

ComPtr<ID3D12Resource> RendererDX12::CreateD3DResourceOnUploadHeap(uint64 inSize)
{
	// Try cache first
	ResourceCache::iterator i = mResourceCache.find(inSize);
	if (i != mResourceCache.end() && !i->second.empty())
	{
		ComPtr<ID3D12Resource> resource = i->second.back();
		i->second.pop_back();
		return resource;
	}

	return CreateD3DResource(D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, inSize);
}

void RendererDX12::RecycleD3DResourceOnUploadHeap(ID3D12Resource *inResource, uint64 inSize)
{
	if (!mIsExiting)
		mDelayCached[mFrameIndex][inSize].push_back(inResource);
}

void RendererDX12::RecycleD3DObject(ID3D12Object *inResource)
{
	if (!mIsExiting)
		mDelayReleased[mFrameIndex].push_back(inResource);
}

#ifndef JPH_ENABLE_VULKAN
Renderer *Renderer::sCreate()
{
	return new RendererDX12;
}
#endif
