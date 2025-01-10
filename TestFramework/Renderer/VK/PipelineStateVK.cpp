// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/VK/PipelineStateVK.h>
#include <Renderer/VK/RendererVK.h>
#include <Renderer/VK/FatalErrorIfFailedVK.h>

PipelineStateVK::PipelineStateVK(RendererVK *inRenderer, const VertexShaderVK *inVertexShader, const EInputDescription *inInputDescription, uint inInputDescriptionCount, const PixelShaderVK *inPixelShader, EDrawPass inDrawPass, EFillMode inFillMode, ETopology inTopology, EDepthTest inDepthTest, EBlendMode inBlendMode, ECullMode inCullMode) :
	mRenderer(inRenderer),
	mVertexShader(inVertexShader),
	mPixelShader(inPixelShader)
{
	VkPipelineShaderStageCreateInfo shader_stages[] = { inVertexShader->mStageInfo, inPixelShader->mStageInfo };

	// TODO: This doesn't follow the SPIR-V alignment rules
	Array<VkVertexInputAttributeDescription> attribute_descriptions;
	VkVertexInputAttributeDescription temp_vtx = { }, temp_instance = { };
	temp_instance.binding = 1;
	uint instance_alignment = 1;
	for (uint i = 0; i < inInputDescriptionCount; ++i)
		switch (inInputDescription[i])
		{
		case EInputDescription::Position:
		case EInputDescription::Normal:
			temp_vtx.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions.push_back(temp_vtx);
			temp_vtx.offset += 3 * sizeof(float);
			break;

		case EInputDescription::Color:
			temp_vtx.format = VK_FORMAT_R8G8B8A8_UNORM;
			attribute_descriptions.push_back(temp_vtx);
			temp_vtx.offset += 4 * sizeof(uint8);
			break;

		case EInputDescription::TexCoord:
			temp_vtx.format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions.push_back(temp_vtx);
			temp_vtx.offset += 2 * sizeof(float);
			break;

		case EInputDescription::InstanceColor:
			instance_alignment = max(instance_alignment, 4u);
			temp_instance.format = VK_FORMAT_R8G8B8A8_UNORM;
			attribute_descriptions.push_back(temp_instance);
			temp_instance.offset += 4 * sizeof(uint8);
			break;

		case EInputDescription::InstanceTransform:
		case EInputDescription::InstanceInvTransform:
			instance_alignment = max(instance_alignment, 16u);
			temp_instance.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			for (int j = 0; j < 4; ++j)
			{
				attribute_descriptions.push_back(temp_instance);
				temp_instance.offset += 4 * sizeof(float);
			}
			break;
		}

	for (uint32 i = 0; i < uint32(attribute_descriptions.size()); ++i)
		attribute_descriptions[i].location = i;

	VkVertexInputBindingDescription binding_description[2];
	binding_description[0].binding = 0;
	binding_description[0].stride = temp_vtx.offset;
	binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	binding_description[1].binding = 1;
	binding_description[1].stride = AlignUp(temp_instance.offset, instance_alignment);
	binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = temp_instance.offset > 0? 2 : 1;
	vertex_input_info.pVertexBindingDescriptions = binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = uint32(attribute_descriptions.size());
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = inTopology == ETopology::Triangle? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = inFillMode == EFillMode::Solid? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = inCullMode == ECullMode::Backface? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = inDepthTest == EDepthTest::On? VK_TRUE : VK_FALSE;
	depth_stencil.depthWriteEnable = inDepthTest == EDepthTest::On? VK_TRUE : VK_FALSE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_GREATER; // Reverse-Z, greater is closer

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	switch (inBlendMode)
	{
	case EBlendMode::Write:
		color_blend_attachment.blendEnable = VK_FALSE;
		break;

	case EBlendMode::AlphaBlend:
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		break;
	}

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = std::size(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = std::size(shader_stages);
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = mRenderer->GetPipelineLayout();
	pipeline_info.renderPass = inDrawPass == EDrawPass::Normal? mRenderer->GetRenderPass() : mRenderer->GetRenderPassShadow();
	FatalErrorIfFailed(vkCreateGraphicsPipelines(mRenderer->GetDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &mGraphicsPipeline));
}

PipelineStateVK::~PipelineStateVK()
{
	vkDeviceWaitIdle(mRenderer->GetDevice());

	vkDestroyPipeline(mRenderer->GetDevice(), mGraphicsPipeline, nullptr);
}

void PipelineStateVK::Activate()
{
	vkCmdBindPipeline(mRenderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}
