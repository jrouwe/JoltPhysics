// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/TextureVK.h>
#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>
#include <Image/BlitSurface.h>

TextureVK::TextureVK(RendererVK *inRenderer, const Surface *inSurface) :
	Texture(inSurface->GetWidth(), inSurface->GetHeight()),
	mRenderer(inRenderer)
{
	ESurfaceFormat format = inSurface->GetFormat();
	VkFormat vk_format = VK_FORMAT_B8G8R8A8_UNORM;
	switch (format)
	{
	case ESurfaceFormat::A4L4:			vk_format = VK_FORMAT_R8G8_UNORM;				format = ESurfaceFormat::A8L8; break;
	case ESurfaceFormat::L8:			vk_format = VK_FORMAT_R8_UNORM;					break;
	case ESurfaceFormat::A8:			vk_format = VK_FORMAT_A8_UNORM_KHR;				break;
	case ESurfaceFormat::A8L8:			vk_format = VK_FORMAT_R8G8_UNORM;				break;
	case ESurfaceFormat::R5G6B5:		vk_format = VK_FORMAT_B5G6R5_UNORM_PACK16;		break;
	case ESurfaceFormat::X1R5G5B5:		vk_format = VK_FORMAT_B5G5R5A1_UNORM_PACK16;	format = ESurfaceFormat::A1R5G5B5; break;
	case ESurfaceFormat::X4R4G4B4:		vk_format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;	format = ESurfaceFormat::A4R4G4B4; break;
	case ESurfaceFormat::A1R5G5B5:		vk_format = VK_FORMAT_B5G5R5A1_UNORM_PACK16;	break;
	case ESurfaceFormat::A4R4G4B4:		vk_format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;	break;
	case ESurfaceFormat::R8G8B8:		vk_format = VK_FORMAT_B8G8R8_UNORM;				break;
	case ESurfaceFormat::B8G8R8:		vk_format = VK_FORMAT_B8G8R8_UNORM;				break;
	case ESurfaceFormat::X8R8G8B8:		vk_format = VK_FORMAT_B8G8R8A8_UNORM;			format = ESurfaceFormat::A8R8G8B8; break;
	case ESurfaceFormat::X8B8G8R8:		vk_format = VK_FORMAT_B8G8R8A8_UNORM;			format = ESurfaceFormat::X8R8G8B8; break;
	case ESurfaceFormat::A8R8G8B8:		vk_format = VK_FORMAT_B8G8R8A8_UNORM;			break;
	case ESurfaceFormat::A8B8G8R8:		vk_format = VK_FORMAT_B8G8R8A8_UNORM;			format = ESurfaceFormat::A8R8G8B8; break;
	case ESurfaceFormat::Invalid:
	default:							JPH_ASSERT(false);								break;
	}

	// Blit the surface to another temporary surface if the format changed
	const Surface *surface = inSurface;
	Ref<Surface> tmp;
	if (format != inSurface->GetFormat())
	{
		tmp = new SoftwareSurface(mWidth, mHeight, format);
		BlitSurface(inSurface, tmp);
		surface = tmp;
	}

	int bpp = surface->GetBytesPerPixel();
	VkDeviceSize image_size = VkDeviceSize(mWidth) * mHeight * bpp;
	VkDevice device = mRenderer->GetDevice();

	BufferVK staging_buffer;
	mRenderer->CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer);

	// Copy data to upload texture
	surface->Lock(ESurfaceLockMode::Read);
	void *data;
	vkMapMemory(device, staging_buffer.mMemory, staging_buffer.mOffset, image_size, 0, &data);
	for (int y = 0; y < mHeight; ++y)
		memcpy(reinterpret_cast<uint8 *>(data) + y * mWidth * bpp, surface->GetData() + y * surface->GetStride(), mWidth * bpp);
	vkUnmapMemory(device, staging_buffer.mMemory);
	surface->UnLock();

	// Create destination image
	mRenderer->CreateImage(mWidth, mHeight, vk_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);

	VkCommandBuffer command_buffer = mRenderer->StartTempCommandBuffer();

	// Make the image suitable for transferring to
	TransitionImageLayout(command_buffer, mImage, vk_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy the data to the destination image
	VkBufferImageCopy region = {};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = { uint32(mWidth), uint32(mHeight), 1 };
	vkCmdCopyBufferToImage(command_buffer, staging_buffer.mBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// Make the image suitable for sampling
	TransitionImageLayout(command_buffer, mImage, vk_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	mRenderer->EndTempCommandBuffer(command_buffer);

	// Destroy temporary buffer
	mRenderer->FreeBuffer(staging_buffer);

	CreateImageViewAndDescriptorSet(vk_format, VK_IMAGE_ASPECT_COLOR_BIT, mRenderer->GetTextureSamplerRepeat());
}

TextureVK::TextureVK(RendererVK *inRenderer, int inWidth, int inHeight) :
	Texture(inWidth, inHeight),
	mRenderer(inRenderer)
{
	VkFormat vk_format = mRenderer->FindDepthFormat();

	// Create render target
	mRenderer->CreateImage(mWidth, mHeight, vk_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);

	CreateImageViewAndDescriptorSet(vk_format, VK_IMAGE_ASPECT_DEPTH_BIT, mRenderer->GetTextureSamplerShadow());
}

TextureVK::~TextureVK()
{
	if (mImage != VK_NULL_HANDLE)
	{
		VkDevice device = mRenderer->GetDevice();

		vkDeviceWaitIdle(device);

		vkDestroyImageView(device, mImageView, nullptr);

		mRenderer->DestroyImage(mImage, mImageMemory);
	}
}

void TextureVK::Bind() const
{
	if (mDescriptorSet != VK_NULL_HANDLE)
		vkCmdBindDescriptorSets(mRenderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderer->GetPipelineLayout(), 1, 1, &mDescriptorSet, 0, nullptr);
}

void TextureVK::CreateImageViewAndDescriptorSet(VkFormat inFormat, VkImageAspectFlags inAspectFlags, VkSampler inSampler)
{
	VkDevice device = mRenderer->GetDevice();

	// Create image view
	mImageView = mRenderer->CreateImageView(mImage, inFormat, inAspectFlags);

	// Allocate descriptor set for binding the texture
	VkDescriptorSetLayout layout = mRenderer->GetDescriptorSetLayoutTexture();
	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
	descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_alloc_info.descriptorPool = mRenderer->GetDescriptorPool();
	descriptor_set_alloc_info.descriptorSetCount = 1;
	descriptor_set_alloc_info.pSetLayouts = &layout;
	FatalErrorIfFailed(vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, &mDescriptorSet));

	VkDescriptorImageInfo image_info = {};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = mImageView;
	image_info.sampler = inSampler;

	VkWriteDescriptorSet descriptor_write = {};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = mDescriptorSet;
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pImageInfo = &image_info;
	vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);
}

void TextureVK::TransitionImageLayout(VkCommandBuffer inCommandBuffer, VkImage inImage, VkFormat inFormat, VkImageLayout inOldLayout, VkImageLayout inNewLayout)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = inOldLayout;
	barrier.newLayout = inNewLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = inImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;

	if (inOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && inNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(inCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
	else if (inOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && inNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(inCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}
}
