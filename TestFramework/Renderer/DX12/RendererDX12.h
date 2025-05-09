// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/UnorderedMap.h>
#include <Renderer/Renderer.h>
#include <Renderer/DX12/CommandQueueDX12.h>
#include <Renderer/DX12/DescriptorHeapDX12.h>
#include <Renderer/DX12/ConstantBufferDX12.h>
#include <Renderer/DX12/TextureDX12.h>

/// DirectX 12 renderer
class RendererDX12 : public Renderer
{
public:
	/// Destructor
	virtual							~RendererDX12() override;

	// See: Renderer
	virtual void					Initialize(ApplicationWindow *inWindow) override;
	virtual bool					BeginFrame(const CameraState &inCamera, float inWorldScale) override;
	virtual void					EndShadowPass() override;
	virtual void					EndFrame() override;
	virtual void					SetProjectionMode() override;
	virtual void					SetOrthoMode() override;
	virtual Ref<Texture>			CreateTexture(const Surface *inSurface) override;
	virtual Ref<VertexShader>		CreateVertexShader(const char *inName) override;
	virtual Ref<PixelShader>		CreatePixelShader(const char *inName) override;
	virtual unique_ptr<PipelineState> CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode) override;
	virtual RenderPrimitive *		CreateRenderPrimitive(PipelineState::ETopology inType) override;
	virtual RenderInstances *		CreateRenderInstances() override;
	virtual Texture *				GetShadowMap() const override		{ return mShadowMap.GetPtr(); }
	virtual void					OnWindowResize() override;

	/// Create a constant buffer
	unique_ptr<ConstantBufferDX12>	CreateConstantBuffer(uint inBufferSize);

	/// Create a buffer on the default heap (usable for permanent buffers)
	ComPtr<ID3D12Resource>			CreateD3DResourceOnDefaultHeap(const void *inData, uint64 inSize);

	/// Create buffer on the upload heap (usable for temporary buffers).
	ComPtr<ID3D12Resource>			CreateD3DResourceOnUploadHeap(uint64 inSize);

	/// Recycle a buffer on the upload heap. This puts it back in a cache and will reuse it when it is certain the GPU is no longer referencing it.
	void							RecycleD3DResourceOnUploadHeap(ID3D12Resource *inResource, uint64 inSize);

	/// Keeps a reference to the resource until the current frame has finished
	void							RecycleD3DObject(ID3D12Object *inResource);

	/// Access to the most important DirectX structures
	ID3D12Device *					GetDevice()							{ return mDevice.Get(); }
	ID3D12RootSignature *			GetRootSignature()					{ return mRootSignature.Get(); }
	ID3D12GraphicsCommandList *		GetCommandList()					{ JPH_ASSERT(mInFrame); return mCommandList.Get(); }
	CommandQueueDX12 &				GetUploadQueue()					{ return mUploadQueue; }
	DescriptorHeapDX12 &			GetDSVHeap()						{ return mDSVHeap; }
	DescriptorHeapDX12 &			GetSRVHeap()						{ return mSRVHeap; }

private:
	// Wait for pending GPU work to complete
	void							WaitForGpu();

	// Create render targets and their views
	void							CreateRenderTargets();

	// Create a depth buffer for the back buffer
	void							CreateDepthBuffer();

	// Function to create a ID3D12Resource on specified heap with specified state
	ComPtr<ID3D12Resource>			CreateD3DResource(D3D12_HEAP_TYPE inHeapType, D3D12_RESOURCE_STATES inResourceState, uint64 inSize);

	// Copy CPU memory into a ID3D12Resource
	void							CopyD3DResource(ID3D12Resource *inDest, const void *inSrc, uint64 inSize);

	// Copy a CPU resource to a GPU resource
	void							CopyD3DResource(ID3D12Resource *inDest, ID3D12Resource *inSrc, uint64 inSize);

	// DirectX interfaces
	ComPtr<IDXGIFactory4>			mDXGIFactory;
	ComPtr<ID3D12Device>			mDevice;
	DescriptorHeapDX12				mRTVHeap;							///< Render target view heap
	DescriptorHeapDX12				mDSVHeap;							///< Depth stencil view heap
	DescriptorHeapDX12				mSRVHeap;							///< Shader resource view heap
	ComPtr<IDXGISwapChain3>			mSwapChain;
	ComPtr<ID3D12Resource>			mRenderTargets[cFrameCount];		///< Two render targets (we're double buffering in order for the CPU to continue while the GPU is rendering)
	D3D12_CPU_DESCRIPTOR_HANDLE		mRenderTargetViews[cFrameCount];	///< The two render views corresponding to the render targets
	ComPtr<ID3D12Resource>			mDepthStencilBuffer;				///< The main depth buffer
	D3D12_CPU_DESCRIPTOR_HANDLE		mDepthStencilView { 0 };			///< A view for binding the depth buffer
	ComPtr<ID3D12CommandAllocator>	mCommandAllocators[cFrameCount];	///< Two command allocator lists (one per frame)
	ComPtr<ID3D12CommandQueue>		mCommandQueue;						///< The command queue that will execute commands (there's only 1 since we want to finish rendering 1 frame before moving onto the next)
	ComPtr<ID3D12GraphicsCommandList> mCommandList;						///< The command list
	ComPtr<ID3D12RootSignature>		mRootSignature;						///< The root signature, we have a simple application so we only need 1, which is suitable for all our shaders
	Ref<TextureDX12>				mShadowMap;							///< Used to render shadow maps
	CommandQueueDX12				mUploadQueue;						///< Queue used to upload resources to GPU memory
	unique_ptr<ConstantBufferDX12>	mVertexShaderConstantBufferProjection[cFrameCount];
	unique_ptr<ConstantBufferDX12>	mVertexShaderConstantBufferOrtho[cFrameCount];
	unique_ptr<ConstantBufferDX12>	mPixelShaderConstantBuffer[cFrameCount];

	// Synchronization objects used to finish rendering and swapping before reusing a command queue
	HANDLE							mFenceEvent;						///< Fence event to wait for the previous frame rendering to complete (in order to free 1 of the buffers)
	ComPtr<ID3D12Fence>				mFence;								///< Fence object, used to signal the end of a frame
	UINT64							mFenceValues[cFrameCount] = {};		///< Values that were used to signal completion of one of the two frames

	using ResourceCache = UnorderedMap<uint64, Array<ComPtr<ID3D12Resource>>>;

	ResourceCache					mResourceCache;						///< Cache items ready to be reused
	ResourceCache					mDelayCached[cFrameCount];			///< List of reusable ID3D12Resources that are potentially referenced by the GPU so can be used only when the GPU finishes
	Array<ComPtr<ID3D12Object>>		mDelayReleased[cFrameCount];		///< Objects that are potentially referenced by the GPU so can only be freed when the GPU finishes
	bool							mIsExiting = false;					///< When exiting we don't want to add references too buffers
};
