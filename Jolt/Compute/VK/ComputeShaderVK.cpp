// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_VK

#include <Jolt/Compute/VK/ComputeShaderVK.h>

JPH_NAMESPACE_BEGIN

ComputeShaderVK::~ComputeShaderVK()
{
	if (mShaderModule != VK_NULL_HANDLE)
		vkDestroyShaderModule(mDevice, mShaderModule, nullptr);

	if (mDescriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

	if (mPipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);

	if (mPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(mDevice, mPipeline, nullptr);
}

bool ComputeShaderVK::Initialize(const Array<uint8> &inSPVCode, VkBuffer inDummyBuffer, ComputeShaderResult &outResult)
{
	const uint32 *spv_words = reinterpret_cast<const uint32 *>(inSPVCode.data());
	size_t spv_word_count = inSPVCode.size() / sizeof(uint32);

	// Minimal SPIR-V parser to extract name to binding info
	UnorderedMap<uint32, String> id_to_name;
	UnorderedMap<uint32, uint32> id_to_binding;
	UnorderedMap<uint32, VkDescriptorType> id_to_descriptor_type;
	UnorderedMap<uint32, uint32> pointer_to_pointee;
	UnorderedMap<uint32, uint32> var_to_ptr_type;
	size_t i = 5; // Skip 5 word header
	while (i < spv_word_count)
	{
		// Parse next word
		uint32 word = spv_words[i];
		uint16 opcode = uint16(word & 0xffff);
		uint16 word_count = uint16(word >> 16);
		if (word_count == 0 || i + word_count > spv_word_count)
			break;

		switch (opcode)
		{
		case 5: // OpName
			if (word_count >= 2)
			{
				uint32 target_id = spv_words[i + 1];
				const char* name = reinterpret_cast<const char*>(&spv_words[i + 2]);
				if (*name != 0)
					id_to_name.insert({ target_id, name });
			}
			break;

		case 16: // OpExecutionMode
			if (word_count >= 6)
			{
				uint32 execution_mode = spv_words[i + 2];
				if (execution_mode == 17) // LocalSize
				{
					// Assert that the group size provided matches the one in the shader
					JPH_ASSERT(GetGroupSizeX() == spv_words[i + 3], "Group size X mismatch");
					JPH_ASSERT(GetGroupSizeY() == spv_words[i + 4], "Group size Y mismatch");
					JPH_ASSERT(GetGroupSizeZ() == spv_words[i + 5], "Group size Z mismatch");
				}
			}
			break;

		case 32: // OpTypePointer
			if (word_count >= 4)
			{
				uint32 result_id = spv_words[i + 1];
				uint32 type_id = spv_words[i + 3];
				pointer_to_pointee.insert({ result_id, type_id });
			}
			break;

		case 59: // OpVariable
			if (word_count >= 3)
			{
				uint32 ptr_type_id = spv_words[i + 1];
				uint32 result_id = spv_words[i + 2];
				var_to_ptr_type.insert({ result_id, ptr_type_id });
			}
			break;

		case 71: // OpDecorate
			if (word_count >= 3)
			{
				uint32 target_id = spv_words[i + 1];
				uint32 decoration = spv_words[i + 2];
				if (decoration == 2) // Block
				{
					id_to_descriptor_type.insert({ target_id, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER });
				}
				else if (decoration == 3) // BufferBlock
				{
					id_to_descriptor_type.insert({ target_id, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER });
				}
				else if (decoration == 33 && word_count >= 4) // Binding
				{
					uint32 binding = spv_words[i + 3];
					id_to_binding.insert({ target_id, binding });
				}
			}
			break;

		default:
			break;
		}

		i += word_count;
	}

	// Build name to binding map
	UnorderedMap<String, std::pair<uint32, VkDescriptorType>> name_to_binding;
	for (const UnorderedMap<uint32, uint32>::value_type &entry : id_to_binding)
	{
		uint32 target_id = entry.first;
		uint32 binding = entry.second;

		// Get the name of the variable
		UnorderedMap<uint32, String>::const_iterator it_name = id_to_name.find(target_id);
		if (it_name != id_to_name.end())
		{
			// Find variable that links to the target
			UnorderedMap<uint32, uint32>::const_iterator it_var_ptr = var_to_ptr_type.find(target_id);
			if (it_var_ptr != var_to_ptr_type.end())
			{
				// Find type pointed at
				uint32 ptr_type = it_var_ptr->second;
				UnorderedMap<uint32, uint32>::const_iterator it_pointee = pointer_to_pointee.find(ptr_type);
				if (it_pointee != pointer_to_pointee.end())
				{
					uint32 pointee_type = it_pointee->second;

					// Find descriptor type
					UnorderedMap<uint32, VkDescriptorType>::iterator it_descriptor_type = id_to_descriptor_type.find(pointee_type);
					VkDescriptorType descriptor_type = it_descriptor_type != id_to_descriptor_type.end() ? it_descriptor_type->second : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

					name_to_binding.insert({ it_name->second, { binding, descriptor_type } });
					continue;
				}
			}
		}
	}

	// Create layout bindings and buffer infos
	if (!name_to_binding.empty())
	{
		mLayoutBindings.reserve(name_to_binding.size());
		mBufferInfos.reserve(name_to_binding.size());

		mBindingNames.reserve(name_to_binding.size());
		for (const UnorderedMap<String, std::pair<uint32, VkDescriptorType>>::value_type &b : name_to_binding)
		{
			const String &name = b.first;
			uint binding = b.second.first;
			VkDescriptorType descriptor_type = b.second.second;

			VkDescriptorSetLayoutBinding l = {};
			l.binding = binding;
			l.descriptorCount = 1;
			l.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			l.descriptorType = descriptor_type;
			mLayoutBindings.push_back(l);

			mBindingNames.push_back(name); // Add all strings to a pool to keep them alive
			mNameToBufferInfoIndex[string_view(mBindingNames.back())] = (uint32)mBufferInfos.size();

			VkDescriptorBufferInfo bi = {};
			bi.offset = 0;
			bi.range = VK_WHOLE_SIZE;
			bi.buffer = inDummyBuffer; // Avoid: The Vulkan spec states: If the nullDescriptor feature is not enabled, buffer must not be VK_NULL_HANDLE
			mBufferInfos.push_back(bi);
		}

		// Create descriptor set layout
		VkDescriptorSetLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = (uint32)mLayoutBindings.size();
		layout_info.pBindings = mLayoutBindings.data();
		if (VKFailed(vkCreateDescriptorSetLayout(mDevice, &layout_info, nullptr, &mDescriptorSetLayout), outResult))
			return false;
	}

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pl_info = {};
	pl_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl_info.setLayoutCount = mDescriptorSetLayout != VK_NULL_HANDLE ? 1 : 0;
	pl_info.pSetLayouts = mDescriptorSetLayout != VK_NULL_HANDLE ? &mDescriptorSetLayout : nullptr;
	if (VKFailed(vkCreatePipelineLayout(mDevice, &pl_info, nullptr, &mPipelineLayout), outResult))
		return false;

	// Create shader module
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = inSPVCode.size();
	create_info.pCode = spv_words;
	if (VKFailed(vkCreateShaderModule(mDevice, &create_info, nullptr, &mShaderModule), outResult))
		return false;

	// Create compute pipeline
	VkComputePipelineCreateInfo pipe_info = {};
	pipe_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipe_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipe_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	pipe_info.stage.module = mShaderModule;
	pipe_info.stage.pName = "main";
	pipe_info.layout = mPipelineLayout;
	if (VKFailed(vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &pipe_info, nullptr, &mPipeline), outResult))
		return false;

	return true;
}

uint32 ComputeShaderVK::NameToBufferInfoIndex(const char *inName) const
{
	UnorderedMap<string_view, uint>::const_iterator it = mNameToBufferInfoIndex.find(inName);
	JPH_ASSERT(it != mNameToBufferInfoIndex.end());
	return it->second;
}

JPH_NAMESPACE_END

#endif // JPH_USE_VK
