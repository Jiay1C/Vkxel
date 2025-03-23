//
// Created by jiayi on 3/22/2025.
//

#ifndef VKXEL_PIPELINE_H
#define VKXEL_PIPELINE_H

#include <vector>

#include "vulkan/vulkan_core.h"


namespace Vkxel::VkUtil {

    struct GraphicsPipeline {
        VkDevice device = nullptr;
        VkPipeline pipeline = nullptr;
        VkPipelineLayout layout = nullptr;
        VkGraphicsPipelineCreateInfo createInfo = {};
        VkPipelineLayoutCreateInfo layoutCreateInfo = {};

        void Destroy();
    };

    struct ComputePipeline {
        VkDevice device = nullptr;
        VkPipeline pipeline = nullptr;
        VkPipelineLayout layout = nullptr;
        VkComputePipelineCreateInfo createInfo = {};
        VkPipelineLayoutCreateInfo layoutCreateInfo = {};

        void Destroy();
    };

    class GraphicsPipelineBuilder {
    public:
        explicit GraphicsPipelineBuilder(const VkDevice device) : _device(device) {}

        GraphicsPipeline Build() const;

        GraphicsPipelineBuilder &SetShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> &shaderStages);
        GraphicsPipelineBuilder &SetPipelineLayout(const std::vector<VkDescriptorSetLayout> &pipelineLayout);
        GraphicsPipelineBuilder &SetVertexInputState(const VkPipelineVertexInputStateCreateInfo &vertexInputInfo);
        GraphicsPipelineBuilder &SetInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo &inputAssembly);
        GraphicsPipelineBuilder &SetViewportState(const VkPipelineViewportStateCreateInfo &viewportState);
        GraphicsPipelineBuilder &SetRasterizationState(const VkPipelineRasterizationStateCreateInfo &rasterizer);
        GraphicsPipelineBuilder &SetMultisampleState(const VkPipelineMultisampleStateCreateInfo &multisampling);
        GraphicsPipelineBuilder &SetDepthStencilState(const VkPipelineDepthStencilStateCreateInfo &depthStencil);
        GraphicsPipelineBuilder &SetColorBlendState(const VkPipelineColorBlendStateCreateInfo &colorBlending);
        GraphicsPipelineBuilder &SetDynamicState(const VkPipelineDynamicStateCreateInfo &dynamicState);
        GraphicsPipelineBuilder &SetDynamicRendering(const VkPipelineRenderingCreateInfo &pipelineRendering);

    protected:
        VkDevice _device = nullptr;
        std::vector<VkPipelineShaderStageCreateInfo> _shaderStages{};
        std::vector<VkDescriptorSetLayout> _descriptorSetLayouts{};

        VkPipelineVertexInputStateCreateInfo _vertexInputInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .vertexBindingDescriptionCount = 0,
                .pVertexBindingDescriptions = nullptr,
                .vertexAttributeDescriptionCount = 0,
                .pVertexAttributeDescriptions = nullptr};

        VkPipelineInputAssemblyStateCreateInfo _inputAssembly{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE};

        VkPipelineViewportStateCreateInfo _viewportState{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                                         .pNext = nullptr,
                                                         .flags = 0,
                                                         .viewportCount = 1,
                                                         .pViewports = nullptr,
                                                         .scissorCount = 1,
                                                         .pScissors = nullptr};

        VkPipelineRasterizationStateCreateInfo _rasterizer{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f};

        VkPipelineMultisampleStateCreateInfo _multisampling{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE};

        VkPipelineDepthStencilStateCreateInfo _depthStencil{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .front = {},
                .back = {},
                .minDepthBounds = 0.0f,
                .maxDepthBounds = 1.0f};

        VkPipelineColorBlendStateCreateInfo _colorBlending{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 0,
                .pAttachments = nullptr,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

        VkPipelineDynamicStateCreateInfo _dynamicState{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                       .pNext = nullptr,
                                                       .flags = 0,
                                                       .dynamicStateCount = 0,
                                                       .pDynamicStates = nullptr};

        VkPipelineRenderingCreateInfo _pipelineRendering{.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                                         .pNext = nullptr,
                                                         .viewMask = 0,
                                                         .colorAttachmentCount = 0,
                                                         .pColorAttachmentFormats = nullptr,
                                                         .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
                                                         .stencilAttachmentFormat = VK_FORMAT_UNDEFINED};
    };

    class DefaultGraphicsPipelineBuilder {
    public:
        explicit DefaultGraphicsPipelineBuilder(const VkDevice device) : _device(device) {}
        GraphicsPipeline Build(const std::vector<VkDescriptorSetLayout> &pipelineLayout) const;

    private:
        VkDevice _device = nullptr;
    };

    class ComputePipelineBuilder {
    public:
        explicit ComputePipelineBuilder(const VkDevice device) : _device(device) {}

        ComputePipeline Build() const;

        ComputePipelineBuilder &SetShader(const VkShaderModule &shader);
        ComputePipelineBuilder &SetShaderName(const std::string &name);
        ComputePipelineBuilder &SetPipelineLayout(const std::vector<VkDescriptorSetLayout> &pipelineLayout);

    private:
        VkDevice _device = nullptr;
        VkShaderModule _shader = nullptr;
        std::string _shader_name = {};
        std::vector<VkDescriptorSetLayout> _descriptorSetLayouts = {};
    };

} // namespace Vkxel::VkUtil
// Vkxel

#endif // VKXEL_PIPELINE_H
