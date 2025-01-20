//
// Created by jiayi on 1/18/2025.
//

#include <array>
#include <vector>
#include <utility>

#include "vulkan/vulkan.h"
#include "VkBootstrap.h"

#include "renderer.h"
#include "application.h"
#include "check.h"
#include "shader.h"
#include "glm/glm.hpp"


namespace Vkxel {
    void Renderer::Init() {

        // Create Instance
        vkb::InstanceBuilder instance_builder;
        auto instance_result = instance_builder
            .set_app_name(Application::Name.data())
            .set_app_version(Application::Version)
            .require_api_version(Application::VulkanVersion)
#ifndef NDEBUG
            .use_default_debug_messenger()
            .request_validation_layers()
#endif
            .build();
        CHECK_NOTNULL_MSG(instance_result, instance_result.error().message());
        _instance = instance_result.value();

        // Create Window
        _window.SetResolution(Application::DefaultResolution.first, Application::DefaultResolution.second)
               .SetTitle(Application::Name)
               .SetInstance(_instance)
               .Create();

        _surface = _window.GetSurface();

        // Select Physical Device
        vkb::PhysicalDeviceSelector physical_device_selector(_instance);
        auto physical_device_result = physical_device_selector
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
            .set_surface(_surface)
            .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
            .select();
        CHECK_NOTNULL_MSG(physical_device_result, physical_device_result.error().message());
        _physical_device = physical_device_result.value();

        // Dynamic Rendering Struct
        VkPhysicalDeviceDynamicRenderingFeatures physical_device_dynamic_rendering_features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
            .dynamicRendering = VK_TRUE
        };

        // Create Device
        vkb::DeviceBuilder device_builder(_physical_device);
        auto device_result = device_builder
            .add_pNext(&physical_device_dynamic_rendering_features)
            .build();
        CHECK_NOTNULL_MSG(device_result, device_result.error().message());

        _device = device_result.value();

        // Create Swapchain
        vkb::SwapchainBuilder swapchain_builder(_device);
        auto swapchain_result = swapchain_builder
            .set_old_swapchain(_swapchain)
            .set_desired_present_mode(Application::DefaultPresentMode)
            .build();
        CHECK_NOTNULL_MSG(swapchain_result, swapchain_result.error().message());
        _swapchain = swapchain_result.value();

        // Get Queue
        auto queue_result = _device.get_queue(vkb::QueueType::graphics);
        CHECK_NOTNULL_MSG(queue_result, queue_result.error().message());
        _queue = queue_result.value();

        // Create Command Pool
        VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = _device.get_queue_index(vkb::QueueType::graphics).value()
        };

        vkCreateCommandPool(_device, &command_pool_create_info, nullptr, &_command_pool);
    }

    void Renderer::Destroy() {
        vkDestroyCommandPool(_device, _command_pool, nullptr);
        vkb::destroy_swapchain(_swapchain);
        vkb::destroy_device(_device);
        _window.Destroy();
        vkb::destroy_instance(_instance);
    }

    void Renderer::Allocate() {
        // Create Pipeline Layout
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0,
            // TODO: Add Descriptor Set
        };
        vkCreatePipelineLayout(_device, &pipeline_layout_create_info, nullptr, &_pipeline_layout);


        // Create Graphics Pipeline

        VkShaderModule vertex_shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic.vert");
        VkShaderModule fragment_shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic.frag");

        std::array shader_stage_create_info = {
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertex_shader_module,
                .pName = "main",
                .pSpecializationInfo = nullptr
            },
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragment_shader_module,
                .pName = "main",
                .pSpecializationInfo = nullptr
            },
        };


        VkVertexInputBindingDescription vertex_input_binding_description{
            .binding = 0,
            .stride = sizeof(glm::vec3),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        VkVertexInputAttributeDescription vertex_input_attribute_description{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0
        };

        VkPipelineVertexInputStateCreateInfo input_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertex_input_binding_description,
            .vertexAttributeDescriptionCount = 1,
            .pVertexAttributeDescriptions = &vertex_input_attribute_description
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };


        VkViewport viewport{
            .x = 0.0f,
            .y = 1.0f,
            .width = static_cast<float>(_swapchain.extent.width),
            .height = static_cast<float>(_swapchain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        VkRect2D scissor{
            .offset = VkOffset2D{0, 0},
            .extent = _swapchain.extent
        };
        VkPipelineViewportStateCreateInfo viewport_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
        };

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
            .depthCompareOp = VK_COMPARE_OP_ALWAYS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
            // TODO: Add Depth Test
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment_state,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };


        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = 0,
            .pDynamicStates = nullptr
        };

        VkPipelineRenderingCreateInfo pipeline_rendering_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &_swapchain.image_format,
        };

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipeline_rendering_create_info,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(shader_stage_create_info.size()),
            .pStages = shader_stage_create_info.data(),
            .pVertexInputState = &input_state_create_info,
            .pInputAssemblyState = &input_assembly_state_create_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_create_info,
            .pRasterizationState = &rasterization_state_create_info,
            .pMultisampleState = &multisample_state_create_info,
            .pDepthStencilState = &depth_stencil_state_create_info,
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = &dynamic_state_create_info,
            .layout = _pipeline_layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };


        CHECK_RESULT_VK(vkCreateGraphicsPipelines(_device, nullptr, 1, &graphics_pipeline_create_info, nullptr, &_pipeline));

        for (auto& shader_stage: shader_stage_create_info) {
            vkDestroyShaderModule(_device, shader_stage.module, nullptr);
        }



    }

    void Renderer::Release() {
        vkDestroyPipeline(_device, _pipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    }

    bool Renderer::Render() {
        return _window.Update();
    }


} // Vkxel