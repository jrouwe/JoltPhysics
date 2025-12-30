// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeShader.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/IncludeVK.h>
#include <Jolt/Core/UnorderedMap.h>

JPH_NAMESPACE_BEGIN

/// Compute shader handle for Vulkan
class JPH_EXPORT ComputeShaderVK : public ComputeShader
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor / destructor
										ComputeShaderVK(VkDevice inDevice, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) : ComputeShader(inGroupSizeX, inGroupSizeY, inGroupSizeZ), mDevice(inDevice) { }
	virtual								~ComputeShaderVK() override;

	/// Initialize from SPIR-V code
	bool								Initialize(const Array<uint8> &inSPVCode, VkBuffer inDummyBuffer, ComputeShaderResult &outResult);

	/// Get index of parameter in buffer infos
	uint32								NameToBufferInfoIndex(const char *inName) const;

	/// Getters
	VkPipeline							GetPipeline() const							{ return mPipeline; }
	VkPipelineLayout					GetPipelineLayout() const					{ return mPipelineLayout; }
	VkDescriptorSetLayout				GetDescriptorSetLayout() const				{ return mDescriptorSetLayout; }
	const Array<VkDescriptorSetLayoutBinding> &GetLayoutBindings() const			{ return mLayoutBindings; }
	const Array<VkDescriptorBufferInfo> &GetBufferInfos() const						{ return mBufferInfos; }

private:
	VkDevice							mDevice;
	VkShaderModule						mShaderModule = VK_NULL_HANDLE;
	VkPipelineLayout					mPipelineLayout = VK_NULL_HANDLE;
	VkPipeline							mPipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout				mDescriptorSetLayout = VK_NULL_HANDLE;
	Array<String>						mBindingNames;								///< A list of binding names, mNameToBufferInfoIndex points to these strings
	UnorderedMap<string_view, uint32>	mNameToBufferInfoIndex;						///< Binding name to buffer index, using a string_view so we can do find() without an allocation
	Array<VkDescriptorSetLayoutBinding>	mLayoutBindings;
	Array<VkDescriptorBufferInfo>		mBufferInfos;
};

JPH_NAMESPACE_END

#endif // JPH_USE_VK
