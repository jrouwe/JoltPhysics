// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <Renderer/Texture.h>
#include <Renderer/FatalErrorIfFailed.h>
#include <Jolt/Core/Profiler.h>
#include <Utils/ReadData.h>
#include <Utils/Log.h>

#include <d3dcompiler.h>
#include <shellscalingapi.h>
#ifdef JPH_DEBUG
	#include <d3d12sdklayers.h>
#endif

static Renderer *sRenderer = nullptr;

struct VertexShaderConstantBuffer
{
	Mat44		mView;
	Mat44		mProjection;
	Mat44		mLightView;
	Mat44		mLightProjection;
};

struct PixelShaderConstantBuffer
{
	Vec4		mCameraPos;
	Vec4		mLightPos;
};

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		if (sRenderer != nullptr)
			sRenderer->OnWindowResize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

Renderer::~Renderer()
{
	// Ensure that the GPU is no longer referencing resources that are about to be cleaned up by the destructor.
	WaitForGpu();

	// Don't add more stuff to the delay reference list
	mIsExiting = true;

	CloseHandle(mFenceEvent);
}

void Renderer::WaitForGpu()
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

void Renderer::CreateRenterTargets()
{
	// Create render targets and views
	for (uint n = 0; n < cFrameCount; ++n)
	{
		mRenderTargetViews[n] = mRTVHeap.Allocate();

		FatalErrorIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));
		mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, mRenderTargetViews[n]);
	}
}

void Renderer::CreateDepthBuffer()
{
	// Free any previous depth stencil view
	if (mDepthStencilView.ptr != 0)
		mDSVHeap.Free(mDepthStencilView);

	// Free any previous depth stencil buffer
	mDepthStencilBuffer.Reset();

	// Allocate depth stencil buffer
	D3D12_CLEAR_VALUE clear_value = {};
	clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	clear_value.DepthStencil.Depth = 1.0f;
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
	depth_stencil_desc.Width = mWindowWidth;
	depth_stencil_desc.Height = mWindowHeight;
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

void Renderer::Initialize()
{
	// Prevent this window from auto scaling
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = TEXT("TestFrameworkClass");
	wcex.hIconSm = nullptr;
	if (!RegisterClassEx(&wcex))
		FatalError("Failed to register window class");

	// Create window
	RECT rc = { 0, 0, mWindowWidth, mWindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	mhWnd = CreateWindow(TEXT("TestFrameworkClass"), TEXT("TestFramework"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance, nullptr);
	if (!mhWnd)
		FatalError("Failed to create window");

	// Show window
	ShowWindow(mhWnd, SW_SHOW);

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
	FatalErrorIfFailed(mDXGIFactory->MakeWindowAssociation(mhWnd, DXGI_MWA_NO_ALT_ENTER));

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
	swap_chain_desc.BufferDesc.Width = mWindowWidth;
	swap_chain_desc.BufferDesc.Height = mWindowHeight;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.OutputWindow = mhWnd;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.Windowed = TRUE;

	ComPtr<IDXGISwapChain> swap_chain;
	FatalErrorIfFailed(mDXGIFactory->CreateSwapChain(mCommandQueue.Get(), &swap_chain_desc, &swap_chain));
	FatalErrorIfFailed(swap_chain.As(&mSwapChain));
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	CreateRenterTargets();

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

	// Sampler 2: Point filtering, using SampleCmp mode to compare if sampled value <= reference value (for shadows)
	samplers[2] = samplers[0];
	samplers[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplers[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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

	// Store global renderer now that we're done initializing
	sRenderer = this;
}

void Renderer::OnWindowResize()
{
	JPH_ASSERT(!mInFrame);

	// Wait for the previous frame to be rendered
	WaitForGpu();

	// Get new window size
	RECT rc;
	GetClientRect(mhWnd, &rc);
	mWindowWidth = max<LONG>(rc.right - rc.left, 8);
	mWindowHeight = max<LONG>(rc.bottom - rc.top, 8);

	// Free the render targets and views to allow resizing the swap chain
	for (uint n = 0; n < cFrameCount; ++n)
	{
		mRTVHeap.Free(mRenderTargetViews[n]);
		mRenderTargets[n].Reset();
	}

	// Resize the swap chain buffers
	FatalErrorIfFailed(mSwapChain->ResizeBuffers(cFrameCount, mWindowWidth, mWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	// Back buffer index may have changed after the resize (it always seems to go to 0 again)
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	// Since we may have switched frame index and we know everything is done, we need to update the fence value for our other frame as completed
	for (uint n = 0; n < cFrameCount; ++n)
		if (mFrameIndex != n)
			mFenceValues[n] = mFence->GetCompletedValue();

	// Recreate render targets
	CreateRenterTargets();

	// Recreate depth buffer
	CreateDepthBuffer();
}

void Renderer::BeginFrame(const CameraState &inCamera, float inWorldScale)
{
	JPH_PROFILE_FUNCTION();

	// Mark that we're in the frame
	JPH_ASSERT(!mInFrame);
	mInFrame = true;

	// Store state
	mCameraState = inCamera;

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

	// Set the main back buffer as render target
	SetRenderTarget(nullptr);

	// Clear the back buffer.
	const float blue[] = { 0.098f, 0.098f, 0.439f, 1.000f };
	mCommandList->ClearRenderTargetView(mRenderTargetViews[mFrameIndex], blue, 0, nullptr);
	mCommandList->ClearDepthStencilView(mDepthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Light properties
	Vec3 light_pos = inWorldScale * Vec3(250, 250, 250);
	Vec3 light_tgt = Vec3::sZero();
	Vec3 light_up = Vec3(0, 1, 0);
	Vec3 light_fwd = (light_tgt - light_pos).Normalized();
	float light_fov = DegreesToRadians(20.0f);
	float light_near = 1.0f;
	float light_far = 1000.0f;

	// Camera properties
	float camera_fovy = inCamera.mFOVY;
	float camera_aspect = static_cast<float>(GetWindowWidth()) / GetWindowHeight();
	float camera_fovx = 2.0f * ATan(camera_aspect * Tan(0.5f * camera_fovy));
	float camera_near = 0.01f * inWorldScale;
	float camera_far = inCamera.mFarPlane * inWorldScale;

	// Set constants for vertex shader in projection mode
	VertexShaderConstantBuffer *vs = mVertexShaderConstantBufferProjection[mFrameIndex]->Map<VertexShaderConstantBuffer>();

	// Camera projection and view
	vs->mProjection = Mat44::sPerspective(camera_fovy, camera_aspect, camera_near, camera_far);
	Vec3 cam_pos = Vec3(inCamera.mPos - mBaseOffset);
	Vec3 tgt = cam_pos + inCamera.mForward;
	vs->mView = Mat44::sLookAt(cam_pos, tgt, inCamera.mUp);

	// Light projection and view
	vs->mLightProjection = Mat44::sPerspective(light_fov, 1.0f, light_near, light_far);
	vs->mLightView = Mat44::sLookAt(light_pos, light_tgt, light_up);

	mVertexShaderConstantBufferProjection[mFrameIndex]->Unmap();

	// Set constants for vertex shader in ortho mode
	vs = mVertexShaderConstantBufferOrtho[mFrameIndex]->Map<VertexShaderConstantBuffer>();

	// Camera ortho projection and view
	vs->mProjection = Mat44(Vec4(2.0f / mWindowWidth, 0.0f, 0.0f, 0.0f), Vec4(0.0f, -2.0f / mWindowHeight, 0.0f, 0.0f), Vec4(0.0f, 0.0f, -1.0f, 0.0f), Vec4(-1.0f, 1.0f, 0.0f, 1.0f));
	vs->mView = Mat44::sIdentity();

	// Light projection and view are unused in ortho mode
	vs->mLightView = Mat44::sIdentity();
	vs->mLightProjection = Mat44::sIdentity();

	mVertexShaderConstantBufferOrtho[mFrameIndex]->Unmap();

	// Switch to 3d projection mode
	SetProjectionMode();

	// Set constants for pixel shader
	PixelShaderConstantBuffer *ps = mPixelShaderConstantBuffer[mFrameIndex]->Map<PixelShaderConstantBuffer>();
	ps->mCameraPos = Vec4(cam_pos, 0);
	ps->mLightPos = Vec4(light_pos, 0);
	mPixelShaderConstantBuffer[mFrameIndex]->Unmap();

	// Set the pixel shader constant buffer data.
	mPixelShaderConstantBuffer[mFrameIndex]->Bind(1);

	// Calculate camera frustum
	mCameraFrustum = Frustum(cam_pos, inCamera.mForward, inCamera.mUp, camera_fovx, camera_fovy, camera_near, camera_far);

	// Calculate light frustum
	mLightFrustum = Frustum(light_pos, light_fwd, light_up, light_fov, light_fov, light_near, light_far);
}

void Renderer::EndFrame()
{
	JPH_PROFILE_FUNCTION();

	// Mark that we're no longer in the frame
	JPH_ASSERT(mInFrame);
	mInFrame = false;

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

void Renderer::SetProjectionMode()
{
	JPH_ASSERT(mInFrame);

	mVertexShaderConstantBufferProjection[mFrameIndex]->Bind(0);
}

void Renderer::SetOrthoMode()
{
	JPH_ASSERT(mInFrame);

	mVertexShaderConstantBufferOrtho[mFrameIndex]->Bind(0);
}

Ref<Texture> Renderer::CreateTexture(const Surface *inSurface)
{
	return new Texture(this, inSurface);
}

Ref<Texture> Renderer::CreateRenderTarget(int inWidth, int inHeight)
{
	return new Texture(this, inWidth, inHeight);
}

void Renderer::SetRenderTarget(Texture *inRenderTarget)
{
	JPH_ASSERT(mInFrame);

	// Unset the previous render target
	if (mRenderTargetTexture != nullptr)
		mRenderTargetTexture->SetAsRenderTarget(false);
	mRenderTargetTexture = nullptr;

	if (inRenderTarget == nullptr)
	{
		// Set the main back buffer as render target
		mCommandList->OMSetRenderTargets(1, &mRenderTargetViews[mFrameIndex], FALSE, &mDepthStencilView);

		// Set viewport
		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight), 0.0f, 1.0f };
		mCommandList->RSSetViewports(1, &viewport);

		// Set scissor rect
		D3D12_RECT scissor_rect = { 0, 0, static_cast<LONG>(mWindowWidth), static_cast<LONG>(mWindowHeight) };
		mCommandList->RSSetScissorRects(1, &scissor_rect);
	}
	else
	{
		// Use the texture as render target
		inRenderTarget->SetAsRenderTarget(true);
		mRenderTargetTexture = inRenderTarget;
	}
}

ComPtr<ID3DBlob> Renderer::CreateVertexShader(const char *inFileName)
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
	Array<uint8> data = ReadData(inFileName);

	// Compile source
	ComPtr<ID3DBlob> shader_blob, error_blob;
	HRESULT hr = D3DCompile(&data[0],
							(uint)data.size(),
							inFileName,
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

	return shader_blob;
}

ComPtr<ID3DBlob> Renderer::CreatePixelShader(const char *inFileName)
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
	Array<uint8> data = ReadData(inFileName);

	// Compile source
	ComPtr<ID3DBlob> shader_blob, error_blob;
	HRESULT hr = D3DCompile(&data[0],
							(uint)data.size(),
							inFileName,
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

	return shader_blob;
}

unique_ptr<ConstantBuffer> Renderer::CreateConstantBuffer(uint inBufferSize)
{
	return make_unique<ConstantBuffer>(this, inBufferSize);
}

unique_ptr<PipelineState> Renderer::CreatePipelineState(ID3DBlob *inVertexShader, const D3D12_INPUT_ELEMENT_DESC *inInputDescription, uint inInputDescriptionCount, ID3DBlob *inPixelShader, D3D12_FILL_MODE inFillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode)
{
	return make_unique<PipelineState>(this, inVertexShader, inInputDescription, inInputDescriptionCount, inPixelShader, inFillMode, inTopology, inDepthTest, inBlendMode, inCullMode);
}

ComPtr<ID3D12Resource> Renderer::CreateD3DResource(D3D12_HEAP_TYPE inHeapType, D3D12_RESOURCE_STATES inResourceState, uint64 inSize)
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

void Renderer::CopyD3DResource(ID3D12Resource *inDest, const void *inSrc, uint64 inSize)
{
	// Copy data to destination buffer
	void *data;
	D3D12_RANGE range = { 0, 0 }; // We're not going to read
	FatalErrorIfFailed(inDest->Map(0, &range, &data));
	memcpy(data, inSrc, size_t(inSize));
	inDest->Unmap(0, nullptr);
}

void Renderer::CopyD3DResource(ID3D12Resource *inDest, ID3D12Resource *inSrc, uint64 inSize)
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

ComPtr<ID3D12Resource> Renderer::CreateD3DResourceOnDefaultHeap(const void *inData, uint64 inSize)
{
	ComPtr<ID3D12Resource> upload = CreateD3DResourceOnUploadHeap(inSize);
	ComPtr<ID3D12Resource> resource = CreateD3DResource(D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, inSize);
	CopyD3DResource(upload.Get(), inData, inSize);
	CopyD3DResource(resource.Get(), upload.Get(), inSize);
	RecycleD3DResourceOnUploadHeap(upload.Get(), inSize);
	return resource;
}

ComPtr<ID3D12Resource> Renderer::CreateD3DResourceOnUploadHeap(uint64 inSize)
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

void Renderer::RecycleD3DResourceOnUploadHeap(ID3D12Resource *inResource, uint64 inSize)
{
	if (!mIsExiting)
		mDelayCached[mFrameIndex][inSize].push_back(inResource);
}

void Renderer::RecycleD3DObject(ID3D12Object *inResource)
{
	if (!mIsExiting)
		mDelayReleased[mFrameIndex].push_back(inResource);
}
