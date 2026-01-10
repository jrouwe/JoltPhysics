// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Renderer.h>
#include <Renderer/VK/TextureVK.h>
#include <Jolt/Compute/VK/ComputeSystemVKImpl.h>

/// Vulkan renderer
class RendererVK : public Renderer, public ComputeSystemVKImpl
{
public:
	/// Constructor / destructor
									RendererVK();
	virtual							~RendererVK() override;

	// See: Renderer
	virtual void					Initialize(ApplicationWindow *inWindow) override;
	virtual ComputeSystem &			GetComputeSystem() override										{ return *this; }
	virtual bool					BeginFrame(const CameraState &inCamera, float inWorldScale) override;
	virtual void					EndShadowPass() override;
	virtual void					EndFrame() override;
	virtual void					SetProjectionMode() override;
	virtual void					SetOrthoMode() override;
	virtual Ref<Texture>			CreateTexture(const Surface *inSurface) override;
	virtual Ref<VertexShader>		CreateVertexShader(const char *inName) override;
	virtual Ref<PixelShader>		CreatePixelShader(const char *inName) override;
	virtual std::unique_ptr<PipelineState> CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode) override;
	virtual RenderPrimitive *		CreateRenderPrimitive(PipelineState::ETopology inType) override;
	virtual RenderInstances *		CreateRenderInstances() override;
	virtual Texture *				GetShadowMap() const override									{ return mShadowMap.GetPtr(); }
	virtual void					OnWindowResize() override;

	VkDescriptorPool				GetDescriptorPool() const										{ return mDescriptorPool; }
	VkDescriptorSetLayout			GetDescriptorSetLayoutTexture() const							{ return mDescriptorSetLayoutTexture; }
	VkSampler						GetTextureSamplerRepeat() const									{ return mTextureSamplerRepeat; }
	VkSampler						GetTextureSamplerShadow() const									{ return mTextureSamplerShadow; }
	VkRenderPass					GetRenderPassShadow() const										{ return mRenderPassShadow; }
	VkRenderPass					GetRenderPass() const											{ return mRenderPass; }
	VkPipelineLayout				GetPipelineLayout() const										{ return mPipelineLayout; }
	VkCommandBuffer					GetCommandBuffer()												{ JPH_ASSERT(mInFrame); return mCommandBuffers[mFrameIndex]; }
	VkCommandBuffer					StartTempCommandBuffer();
	void							EndTempCommandBuffer(VkCommandBuffer inCommandBuffer);
	void							CopyBuffer(VkBuffer inSrc, VkBuffer inDst, VkDeviceSize inSize);
	void							CreateDeviceLocalBuffer(const void *inData, VkDeviceSize inSize, VkBufferUsageFlags inUsage, BufferVK &outBuffer);
	void							CreateImage(uint32 inWidth, uint32 inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkImage &outImage, MemoryVK &ioMemory);
	void							DestroyImage(VkImage inImage, MemoryVK &ioMemory);
	VkImageView						CreateImageView(VkImage inImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags);
	VkFormat						FindDepthFormat();
	void							FreeBufferDelayed(const BufferVK &inBuffer)						{ mPerFrameFreedBuffers[mFrameIndex].push_back(inBuffer); }

protected:
	// Callbacks from ComputeSystemVKImpl
	virtual void					OnInstanceCreated() override;
	virtual void					GetInstanceExtensions(Array<const char *> &outExtensions) override;
	virtual void					GetDeviceExtensions(Array<const char *> &outExtensions) override;
	virtual void					GetEnabledFeatures(VkPhysicalDeviceFeatures2 &ioFeatures) override;
	virtual bool					HasPresentSupport(VkPhysicalDevice inDevice, uint32 inQueueFamilyIndex) override;
	virtual VkSurfaceFormatKHR		SelectFormat(VkPhysicalDevice inDevice) override;

private:
	void							CreateSwapChain(VkPhysicalDevice inDevice);
	void							DestroySwapChain();
	void							UpdateViewPortAndScissorRect(uint32 inWidth, uint32 inHeight);
	VkSemaphore						AllocateSemaphore();
	void							FreeSemaphore(VkSemaphore inSemaphore);

	VkSurfaceKHR					mSurface = VK_NULL_HANDLE;
	VkSwapchainKHR					mSwapChain = VK_NULL_HANDLE;
	bool							mSubOptimalSwapChain = false;
	Array<VkImage>					mSwapChainImages;
	VkFormat						mSwapChainImageFormat;
	VkExtent2D						mSwapChainExtent;
	Array<VkImageView>				mSwapChainImageViews;
	VkImage							mDepthImage = VK_NULL_HANDLE;
	MemoryVK						mDepthImageMemory;
	VkImageView						mDepthImageView = VK_NULL_HANDLE;
	VkDescriptorSetLayout			mDescriptorSetLayoutUBO = VK_NULL_HANDLE;
	VkDescriptorSetLayout			mDescriptorSetLayoutTexture = VK_NULL_HANDLE;
	VkDescriptorPool				mDescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet					mDescriptorSets[cFrameCount];
	VkDescriptorSet					mDescriptorSetsOrtho[cFrameCount];
	VkSampler						mTextureSamplerShadow = VK_NULL_HANDLE;
	VkSampler						mTextureSamplerRepeat = VK_NULL_HANDLE;
	VkRenderPass					mRenderPassShadow = VK_NULL_HANDLE;
	VkRenderPass					mRenderPass = VK_NULL_HANDLE;
	VkPipelineLayout				mPipelineLayout = VK_NULL_HANDLE;
	VkFramebuffer					mShadowFrameBuffer = VK_NULL_HANDLE;
	Array<VkFramebuffer>			mSwapChainFramebuffers;
	uint32							mImageIndex = 0;
	VkCommandPool					mCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer					mCommandBuffers[cFrameCount];
	Array<VkSemaphore>				mAvailableSemaphores;
	Array<VkSemaphore>				mImageAvailableSemaphores;
	Array<VkSemaphore>				mRenderFinishedSemaphores;
	VkFence							mInFlightFences[cFrameCount];
	Ref<TextureVK>					mShadowMap;
	Ref<ComputeBuffer>				mVertexShaderConstantBufferProjection[cFrameCount];
	Ref<ComputeBuffer>				mVertexShaderConstantBufferOrtho[cFrameCount];
	Ref<ComputeBuffer>				mPixelShaderConstantBuffer[cFrameCount];
	Array<BufferVK>					mPerFrameFreedBuffers[cFrameCount];
};

extern Renderer *CreateRendererVK();
