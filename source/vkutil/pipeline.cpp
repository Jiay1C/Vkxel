//
// Created by jiayi on 3/22/2025.
//

#include <array>

#include "engine/data_type.h"
#include "engine/shader.h"
#include "pipeline.h"

#include "util/application.h"
#include "util/check.h"

namespace Vkxel::VkUtil {

    void GraphicsPipeline::Destroy() {
        if (pipeline) {
            vkDestroyPipeline(device, pipeline, nullptr);
            pipeline = nullptr;
        }
        if (layout) {
            vkDestroyPipelineLayout(device, layout, nullptr);
            layout = nullptr;
        }
    }

    void ComputePipeline::Destroy() {
        if (pipeline) {
            vkDestroyPipeline(device, pipeline, nullptr);
            pipeline = nullptr;
        }
        if (layout) {
            vkDestroyPipelineLayout(device, layout, nullptr);
            layout = nullptr;
        }
    }

    GraphicsPipeline GraphicsPipelineBuilder::Build() const {
        VkPipelineLayoutCreateInfo layout_create_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                      .setLayoutCount =
                                                              static_cast<uint32_t>(_descriptorSetLayouts.size()),
                                                      .pSetLayouts = _descriptorSetLayouts.data(),
                                                      .pushConstantRangeCount = 0,
                                                      .pPushConstantRanges = nullptr};

        VkPipelineLayout pipeline_layout = nullptr;
        CHECK_RESULT_VK(vkCreatePipelineLayout(_device, &layout_create_info, nullptr, &pipeline_layout));

        VkGraphicsPipelineCreateInfo pipeline_create_info{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                          .pNext = &_pipelineRendering,
                                                          .flags = 0,
                                                          .stageCount = static_cast<uint32_t>(_shaderStages.size()),
                                                          .pStages = _shaderStages.data(),
                                                          .pVertexInputState = &_vertexInputInfo,
                                                          .pInputAssemblyState = &_inputAssembly,
                                                          .pViewportState = &_viewportState,
                                                          .pRasterizationState = &_rasterizer,
                                                          .pMultisampleState = &_multisampling,
                                                          .pDepthStencilState = &_depthStencil,
                                                          .pColorBlendState = &_colorBlending,
                                                          .pDynamicState = &_dynamicState,
                                                          .layout = pipeline_layout,
                                                          .basePipelineHandle = VK_NULL_HANDLE,
                                                          .basePipelineIndex = -1};
        VkPipeline pipeline = nullptr;
        CHECK_RESULT_VK(vkCreateGraphicsPipelines(_device, nullptr, 1, &pipeline_create_info, nullptr, &pipeline));

        return {.device = _device,
                .pipeline = pipeline,
                .layout = pipeline_layout,
                .createInfo = pipeline_create_info,
                .layoutCreateInfo = layout_create_info};
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> &shaderStages) {
        _shaderStages = shaderStages;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetPipelineLayout(const std::vector<VkDescriptorSetLayout> &pipelineLayout) {
        _descriptorSetLayouts = pipelineLayout;
        return *this;
    }


    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetVertexInputState(const VkPipelineVertexInputStateCreateInfo &vertexInputInfo) {
        _vertexInputInfo = vertexInputInfo;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo &inputAssembly) {
        _inputAssembly = inputAssembly;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetViewportState(const VkPipelineViewportStateCreateInfo &viewportState) {
        _viewportState = viewportState;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetRasterizationState(const VkPipelineRasterizationStateCreateInfo &rasterizer) {
        _rasterizer = rasterizer;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetMultisampleState(const VkPipelineMultisampleStateCreateInfo &multisampling) {
        _multisampling = multisampling;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetDepthStencilState(const VkPipelineDepthStencilStateCreateInfo &depthStencil) {
        _depthStencil = depthStencil;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetColorBlendState(const VkPipelineColorBlendStateCreateInfo &colorBlending) {
        _colorBlending = colorBlending;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetDynamicState(const VkPipelineDynamicStateCreateInfo &dynamicState) {
        _dynamicState = dynamicState;
        return *this;
    }

    GraphicsPipelineBuilder &
    GraphicsPipelineBuilder::SetDynamicRendering(const VkPipelineRenderingCreateInfo &pipelineRendering) {
        _pipelineRendering = pipelineRendering;
        return *this;
    }

    GraphicsPipeline
    DefaultGraphicsPipelineBuilder::Build(const std::vector<VkDescriptorSetLayout> &pipelineLayout) const {
        GraphicsPipelineBuilder builder(_device);

        builder.SetPipelineLayout(pipelineLayout);

        VkShaderModule shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic");

        builder.SetShaderStages({
                VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                .module = shader_module,
                                                .pName = "vertexMain",
                                                .pSpecializationInfo = nullptr},
                VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                .module = shader_module,
                                                .pName = "fragmentMain",
                                                .pSpecializationInfo = nullptr},
        });

        VkVertexInputBindingDescription vertex_input_binding_description{
                .binding = 0, .stride = sizeof(VertexData), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

        std::array vertex_input_attribute_description = {
                VkVertexInputAttributeDescription{.location = 0,
                                                  .binding = 0,
                                                  .format = VK_FORMAT_R32G32B32_SFLOAT,
                                                  .offset = offsetof(VertexData, position)},
                VkVertexInputAttributeDescription{.location = 1,
                                                  .binding = 0,
                                                  .format = VK_FORMAT_R32G32B32_SFLOAT,
                                                  .offset = offsetof(VertexData, normal)},
                VkVertexInputAttributeDescription{.location = 2,
                                                  .binding = 0,
                                                  .format = VK_FORMAT_R32G32B32_SFLOAT,
                                                  .offset = offsetof(VertexData, color)}};

        builder.SetVertexInputState(
                {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                 .vertexBindingDescriptionCount = 1,
                 .pVertexBindingDescriptions = &vertex_input_binding_description,
                 .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attribute_description.size()),
                 .pVertexAttributeDescriptions = vertex_input_attribute_description.data()});

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                  VK_COLOR_COMPONENT_A_BIT};

        builder.SetColorBlendState({.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                    .logicOpEnable = VK_FALSE,
                                    .logicOp = VK_LOGIC_OP_COPY,
                                    .attachmentCount = 1,
                                    .pAttachments = &color_blend_attachment_state,
                                    .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}});


        std::array dynamic_state = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        builder.SetDynamicState({.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                 .dynamicStateCount = static_cast<uint32_t>(dynamic_state.size()),
                                 .pDynamicStates = dynamic_state.data()});

        builder.SetDynamicRendering({.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                     .colorAttachmentCount = 1,
                                     .pColorAttachmentFormats = &Application::DefaultColorFormat,
                                     .depthAttachmentFormat = Application::DefaultDepthFormat});

        GraphicsPipeline pipeline = builder.Build();

        vkDestroyShaderModule(_device, shader_module, nullptr);

        return pipeline;
    }


    ComputePipelineBuilder &ComputePipelineBuilder::SetShader(const VkShaderModule &shader) {
        _shader = shader;
        return *this;
    }

    ComputePipelineBuilder &ComputePipelineBuilder::SetShaderName(const std::string &name) {
        _shader_name = name;
        return *this;
    }

    ComputePipelineBuilder &
    ComputePipelineBuilder::SetPipelineLayout(const std::vector<VkDescriptorSetLayout> &pipelineLayout) {
        _descriptorSetLayouts = pipelineLayout;
        return *this;
    }

    ComputePipeline ComputePipelineBuilder::Build() const {
        VkPipelineLayoutCreateInfo layout_create_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                      .setLayoutCount =
                                                              static_cast<uint32_t>(_descriptorSetLayouts.size()),
                                                      .pSetLayouts = _descriptorSetLayouts.data(),
                                                      .pushConstantRangeCount = 0};

        VkPipelineLayout pipeline_layout = nullptr;
        CHECK_RESULT_VK(vkCreatePipelineLayout(_device, &layout_create_info, nullptr, &pipeline_layout));

        VkComputePipelineCreateInfo pipeline_create_info{
                .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                .stage = VkPipelineShaderStageCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                         .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                                                         .module = _shader,
                                                         .pName = _shader_name.data(),
                                                         .pSpecializationInfo = nullptr},
                .layout = pipeline_layout};

        VkPipeline pipeline = nullptr;
        CHECK_RESULT_VK(vkCreateComputePipelines(_device, nullptr, 1, &pipeline_create_info, nullptr, &pipeline));

        return {.device = _device,
                .pipeline = pipeline,
                .layout = pipeline_layout,
                .createInfo = pipeline_create_info,
                .layoutCreateInfo = layout_create_info};
    }


} // namespace Vkxel::VkUtil
