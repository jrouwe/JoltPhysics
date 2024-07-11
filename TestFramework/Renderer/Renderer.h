// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/UnorderedMap.h>
#include <Image/Surface.h>
#include <Renderer/Frustum.h>
#include <Renderer/ConstantBuffer.h>
#include <Renderer/PipelineState.h>
#include <Renderer/CommandQueue.h>
#include <Renderer/DescriptorHeap.h>
#include <memory>

// Forward declares
class Texture;

/// Camera setup
struct CameraState
{
									CameraState() : mPos(RVec3::sZero()), mForward(0, 0, -1), mUp(0, 1, 0), mFOVY(DegreesToRadians(70.0f)), mFarPlane(100.0f) { }

	RVec3							mPos;								///< Camera position
	Vec3							mForward;							///< Camera forward vector
	Vec3							mUp;								///< Camera up vector
	float							mFOVY;								///< Field of view in radians in up direction
	float							mFarPlane;							///< Distance of far plane
};

/// Responsible for rendering primitives to the screen
class Renderer
{
public:
	/// Destructor
									~Renderer();

	/// Initialize DirectX
	void							Initialize();

	/// Callback when the window resizes and the back buffer needs to be adjusted
	void							OnWindowResize();

	/// Get window size
	int								GetWindowWidth()					{ return mWindowWidth; }
	int								GetWindowHeight()					{ return mWindowHeight; }

	/// Access to the window handle
	HWND							GetWindowHandle() const				{ return mhWnd; }

	/// Access to the most important DirectX structures
	ID3D12Device *					GetDevice()							{ return mDevice.Get(); }
	ID3D12RootSignature *			GetRootSignature()					{ return mRootSignature.Get(); }
	ID3D12GraphicsCommandList *		GetCommandList()					{ JPH_ASSERT(mInFrame); return mCommandList.Get(); }
	CommandQueue &					GetUploadQueue()					{ return mUploadQueue; }
	DescriptorHeap &				GetDSVHeap()						{ return mDSVHeap; }
	DescriptorHeap &				GetSRVHeap()						{ return mSRVHeap; }

	/// Start / end drawing a frame
	void							BeginFrame(const CameraState &inCamera, float inWorldScale);
	void							EndFrame();

	/// Switch between orthographic and 3D projection mode
	void							SetProjectionMode();
	void							SetOrthoMode();

	/// Create texture from an image surface
	Ref<Texture>					CreateTexture(const Surface *inSurface);

	/// Create a texture to render to (currently depth buffer only)
	Ref<Texture>					CreateRenderTarget(int inWidth, int inHeight);

	/// Change the render target to a texture. Use nullptr to set back to the main render target.
	void							SetRenderTarget(Texture *inRenderTarget);

	/// Compile a vertex shader
	ComPtr<ID3DBlob>				CreateVertexShader(const char *inFileName);

	/// Compile a pixel shader
	ComPtr<ID3DBlob>				CreatePixelShader(const char *inFileName);

	/// Create a constant buffer for the shader
	unique_ptr<ConstantBuffer>		CreateConstantBuffer(uint inBufferSize);

	/// Create pipeline state object that defines the complete state of how primitives should be rendered
	unique_ptr<PipelineState>		CreatePipelineState(ID3DBlob *inVertexShader, const D3D12_INPUT_ELEMENT_DESC *inInputDescription, uint inInputDescriptionCount, ID3DBlob *inPixelShader, D3D12_FILL_MODE inFillMode, D3D12_PRIMITIVE_TOPOLOGY_TYPE inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode);

	/// Get the camera state / frustum (only valid between BeginFrame() / EndFrame())
	const CameraState &				GetCameraState() const				{ JPH_ASSERT(mInFrame); return mCameraState; }
	const Frustum &					GetCameraFrustum() const			{ JPH_ASSERT(mInFrame); return mCameraFrustum; }

	/// Offset relative to which the world is rendered, helps avoiding rendering artifacts at big distances
	RVec3							GetBaseOffset() const				{ return mBaseOffset; }
	void							SetBaseOffset(RVec3 inOffset)		{ mBaseOffset = inOffset; }

	/// Get the light frustum (only valid between BeginFrame() / EndFrame())
	const Frustum &					GetLightFrustum() const				{ JPH_ASSERT(mInFrame); return mLightFrustum; }

	/// How many frames our pipeline is
	static const uint				cFrameCount = 2;

	/// Which frame is currently rendering (to keep track of which buffers are free to overwrite)
	uint							GetCurrentFrameIndex() const		{ JPH_ASSERT(mInFrame); return mFrameIndex; }

	/// Create a buffer on the default heap (usable for permanent buffers)
	ComPtr<ID3D12Resource>			CreateD3DResourceOnDefaultHeap(const void *inData, uint64 inSize);

	/// Create buffer on the upload heap (usable for temporary buffers).
	ComPtr<ID3D12Resource>			CreateD3DResourceOnUploadHeap(uint64 inSize);

	/// Recycle a buffer on the upload heap. This puts it back in a cache and will reuse it when it is certain the GPU is no longer referencing it.
	void							RecycleD3DResourceOnUploadHeap(ID3D12Resource *inResource, uint64 inSize);

	/// Keeps a reference to the resource until the current frame has finished
	void							RecycleD3DObject(ID3D12Object *inResource);

private:
	// Wait for pending GPU work to complete
	void							WaitForGpu();

	// Create render targets and their views
	void							CreateRenterTargets();

	// Create a depth buffer for the back buffer
	void							CreateDepthBuffer();

	// Function to create a ID3D12Resource on specified heap with specified state
	ComPtr<ID3D12Resource>			CreateD3DResource(D3D12_HEAP_TYPE inHeapType, D3D12_RESOURCE_STATES inResourceState, uint64 inSize);

	// Copy CPU memory into a ID3D12Resource
	void							CopyD3DResource(ID3D12Resource *inDest, const void *inSrc, uint64 inSize);

	// Copy a CPU resource to a GPU resource
	void							CopyD3DResource(ID3D12Resource *inDest, ID3D12Resource *inSrc, uint64 inSize);

	HWND							mhWnd;
	int								mWindowWidth = 1920;
	int								mWindowHeight = 1080;
	unique_ptr<ConstantBuffer>		mVertexShaderConstantBufferProjection[cFrameCount];
	unique_ptr<ConstantBuffer>		mVertexShaderConstantBufferOrtho[cFrameCount];
	unique_ptr<ConstantBuffer>		mPixelShaderConstantBuffer[cFrameCount];
	bool							mInFrame = false;					///< If we're within a BeginFrame() / EndFrame() pair
	CameraState						mCameraState;
	RVec3							mBaseOffset { RVec3::sZero() };		///< Offset to subtract from the camera position to deal with large worlds
	Frustum							mCameraFrustum;
	Frustum							mLightFrustum;

	// DirectX interfaces
	ComPtr<IDXGIFactory4>			mDXGIFactory;
	ComPtr<ID3D12Device>			mDevice;
	DescriptorHeap					mRTVHeap;							///< Render target view heap
	DescriptorHeap					mDSVHeap;							///< Depth stencil view heap
	DescriptorHeap					mSRVHeap;							///< Shader resource view heap
	ComPtr<IDXGISwapChain3>			mSwapChain;
	ComPtr<ID3D12Resource>			mRenderTargets[cFrameCount];		///< Two render targets (we're double buffering in order for the CPU to continue while the GPU is rendering)
	D3D12_CPU_DESCRIPTOR_HANDLE		mRenderTargetViews[cFrameCount];	///< The two render views corresponding to the render targets
	ComPtr<ID3D12Resource>			mDepthStencilBuffer;				///< The main depth buffer
	D3D12_CPU_DESCRIPTOR_HANDLE		mDepthStencilView { 0 };			///< A view for binding the depth buffer
	ComPtr<ID3D12CommandAllocator>	mCommandAllocators[cFrameCount];	///< Two command allocator lists (one per frame)
	ComPtr<ID3D12CommandQueue>		mCommandQueue;						///< The command queue that will execute commands (there's only 1 since we want to finish rendering 1 frame before moving onto the next)
	ComPtr<ID3D12GraphicsCommandList> mCommandList;						///< The command list
	ComPtr<ID3D12RootSignature>		mRootSignature;						///< The root signature, we have a simple application so we only need 1, which is suitable for all our shaders
	Ref<Texture>					mRenderTargetTexture;				///< When rendering to a texture, this is the active texture
	CommandQueue					mUploadQueue;						///< Queue used to upload resources to GPU memory

	// Synchronization objects used to finish rendering and swapping before reusing a command queue
	uint							mFrameIndex;						///< Current frame index (0 or 1)
	HANDLE							mFenceEvent;						///< Fence event to wait for the previous frame rendering to complete (in order to free 1 of the buffers)
	ComPtr<ID3D12Fence>				mFence;								///< Fence object, used to signal the end of a frame
	UINT64							mFenceValues[cFrameCount] = {};		///< Values that were used to signal completion of one of the two frames

	using ResourceCache = UnorderedMap<uint64, Array<ComPtr<ID3D12Resource>>>;

	ResourceCache					mResourceCache;						///< Cache items ready to be reused
	ResourceCache					mDelayCached[cFrameCount];			///< List of reusable ID3D12Resources that are potentially referenced by the GPU so can be used only when the GPU finishes
	Array<ComPtr<ID3D12Object>>		mDelayReleased[cFrameCount];		///< Objects that are potentially referenced by the GPU so can only be freed when the GPU finishes
	bool							mIsExiting = false;					///< When exiting we don't want to add references too buffers
};
