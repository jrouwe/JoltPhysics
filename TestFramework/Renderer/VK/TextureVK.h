// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Texture.h>

#include <vulkan/vulkan.h>

class RendererVK;

class TextureVK : public Texture
{
public:
	/// Constructor, called by Renderer::CreateTextureVK
										TextureVK(RendererVK *inRenderer, const Surface *inSurface);	// Create a normal TextureVK
										TextureVK(RendererVK *inRenderer, int inWidth, int inHeight);	// Create a render target (depth only)
	virtual								~TextureVK() override;

	/// Bind texture to the pixel shader
	virtual void						Bind() const override;

	VkImageView							GetImageView() const						{ return mImageView; }

private:
	void								CreateImageViewAndDescriptorSet(VkFormat inFormat, VkImageAspectFlags inAspectFlags, VkSampler inSampler);
	void								TransitionImageLayout(VkCommandBuffer inCommandBuffer, VkImage inImage, VkFormat inFormat, VkImageLayout inOldLayout, VkImageLayout inNewLayout);

	RendererVK *						mRenderer;
	VkImage								mImage = VK_NULL_HANDLE;
	VkDeviceMemory						mImageMemory = VK_NULL_HANDLE;
	VkImageView							mImageView = VK_NULL_HANDLE;
	VkDescriptorSet						mDescriptorSet = VK_NULL_HANDLE;
};
