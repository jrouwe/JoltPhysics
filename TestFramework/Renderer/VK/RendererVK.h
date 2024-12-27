// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Renderer.h>
#include <Renderer/VK/ConstantBufferVK.h>
#include <Renderer/VK/TextureVK.h>

#include <vulkan/vulkan.h>

/// Vulkan renderer
class RendererVK : public Renderer
{
public:
	/// Destructor
	virtual							~RendererVK() override;

	// See: Renderer
	virtual void					Initialize() override;
	virtual void					BeginFrame(const CameraState &inCamera, float inWorldScale) override;
	virtual void					EndShadowPass() override;
	virtual void					EndFrame() override;
	virtual void					SetProjectionMode() override;
	virtual void					SetOrthoMode() override;
	virtual Ref<Texture>			CreateTexture(const Surface *inSurface) override;
	virtual Ref<VertexShader>		CreateVertexShader(const char *inFileName) override;
	virtual Ref<PixelShader>		CreatePixelShader(const char *inFileName) override;
	virtual unique_ptr<PipelineState> CreatePipelineState(const VertexShader *inVertexShader, const PipelineState::EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShader *inPixelShader, PipelineState::EDrawPass inDrawPass, PipelineState::EFillMode inFillMode, PipelineState::ETopology inTopology, PipelineState::EDepthTest inDepthTest, PipelineState::EBlendMode inBlendMode, PipelineState::ECullMode inCullMode) override;
	virtual RenderPrimitive *		CreateRenderPrimitive(PipelineState::ETopology inType) override;
	virtual RenderInstances *		CreateRenderInstances() override;
	virtual Texture *				GetShadowMap() const override									{ return mShadowMap.GetPtr(); }
	virtual void					OnWindowResize() override;

	VkDevice						GetDevice() const												{ return mDevice; }
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
	void							CreateBuffer(VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory);
	void							CopyBuffer(VkBuffer inSrc, VkBuffer inDst, VkDeviceSize inSize);
	void							CreateDeviceLocalBuffer(const void *inData, VkDeviceSize inSize, VkBufferUsageFlags inUsage, VkBuffer &outBuffer, VkDeviceMemory &outBufferMemory);
	void							FreeBuffer(VkBuffer inBuffer, VkDeviceMemory inMemory);
	unique_ptr<ConstantBufferVK>	CreateConstantBuffer(VkDeviceSize inBufferSize);
	void							CreateImage(uint32 inWidth, uint32 inHeight, VkFormat inFormat, VkImageTiling inTiling, VkImageUsageFlags inUsage, VkMemoryPropertyFlags inProperties, VkImage &outImage, VkDeviceMemory &outMemory);
	VkImageView						CreateImageView(VkImage inImage, VkFormat inFormat, VkImageAspectFlags inAspectFlags);
	VkFormat						FindDepthFormat();

private:
	uint32							FindMemoryType(uint32 inTypeFilter, VkMemoryPropertyFlags inProperties);
	VkSurfaceFormatKHR				SelectFormat(VkPhysicalDevice inDevice);
	void							CreateSwapChain(VkPhysicalDevice inDevice);
	void							DestroySwapChain();
	void							UpdateViewPortAndScissorRect(uint32 inWidth, uint32 inHeight);

	struct FreedBuffer
	{
									FreedBuffer() = default;
									FreedBuffer(VkBuffer inBuffer, VkDeviceMemory inMemory) : mBuffer(inBuffer), mMemory(inMemory) { }

		VkBuffer					mBuffer;
		VkDeviceMemory				mMemory;
	};

	VkInstance						mInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT		mDebugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice				mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice						mDevice = VK_NULL_HANDLE;
	uint32							mGraphicsQueueIndex = 0;
	uint32							mPresentQueueIndex = 0;
	VkQueue							mGraphicsQueue = VK_NULL_HANDLE;
	VkQueue							mPresentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR					mSurface = VK_NULL_HANDLE;
	VkSwapchainKHR					mSwapChain = VK_NULL_HANDLE;
	Array<VkImage>					mSwapChainImages;
	VkFormat						mSwapChainImageFormat;
	VkExtent2D						mSwapChainExtent;
	Array<VkImageView>				mSwapChainImageViews;
	VkImage							mDepthImage = VK_NULL_HANDLE;
	VkDeviceMemory					mDepthImageMemory = VK_NULL_HANDLE;
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
	VkSemaphore						mImageAvailableSemaphores[cFrameCount];
	VkSemaphore						mRenderFinishedSemaphores[cFrameCount];
	VkFence							mInFlightFences[cFrameCount];
	Ref<TextureVK>					mShadowMap;
	unique_ptr<ConstantBufferVK>	mVertexShaderConstantBufferProjection[cFrameCount];
	unique_ptr<ConstantBufferVK>	mVertexShaderConstantBufferOrtho[cFrameCount];
	unique_ptr<ConstantBufferVK>	mPixelShaderConstantBuffer[cFrameCount];
	Array<FreedBuffer>				mFreedBuffers[cFrameCount];
};
