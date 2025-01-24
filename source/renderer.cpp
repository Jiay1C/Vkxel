//
// Created by jiayi on 1/18/2025.
//

#include <array>
#include <vector>
#include <utility>

#include "vulkan/vulkan.h"
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "glm/glm.hpp"

#include "renderer.h"
#include "application.h"
#include "check.h"
#include "shader.h"
#include "model.h"


namespace Vkxel {
    void Renderer::Init() {

        // Create Instance
        vkb::InstanceBuilder instance_builder;
        auto instance_result = instance_builder
            .set_app_name(Application::Name.data())
            .set_app_version(Application::Version)
            .require_api_version(Application::VulkanVersion)
            .enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
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
        VkPhysicalDeviceVulkan13Features physical_device_vulkan13_features{
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        };

        vkb::PhysicalDeviceSelector physical_device_selector(_instance);
        auto physical_device_result = physical_device_selector
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
            .set_surface(_surface)
            .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
            .set_required_features_13(physical_device_vulkan13_features)
            .select();
        CHECK_NOTNULL_MSG(physical_device_result, physical_device_result.error().message());
        _physical_device = physical_device_result.value();

        // Create Device
        vkb::DeviceBuilder device_builder(_physical_device);
        auto device_result = device_builder.build();
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

        auto swapchain_image_result = _swapchain.get_images();
        CHECK_NOTNULL_MSG(swapchain_image_result, swapchain_image_result.error().message());
        _swapchain_image = std::move(swapchain_image_result.value());
        auto swapchain_image_view_result = _swapchain.get_image_views();
        CHECK_NOTNULL_MSG(swapchain_image_view_result, swapchain_image_view_result.error().message());
        _swapchain_image_view = std::move(swapchain_image_view_result.value());

        // Get Queue
        auto queue_result = _device.get_queue(vkb::QueueType::graphics);
        CHECK_NOTNULL_MSG(queue_result, queue_result.error().message());
        _queue = queue_result.value();

        // Create Command Pool
        VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = _device.get_queue_index(vkb::QueueType::graphics).value()
        };

        CHECK_RESULT_VK(vkCreateCommandPool(_device, &command_pool_create_info, nullptr, &_command_pool));

        // Create VMA Allocator
        VmaAllocatorCreateInfo vma_allocator_create_info{
            .physicalDevice = _physical_device,
            .device = _device,
            .instance = _instance,
        };

        CHECK_RESULT_VK(vmaCreateAllocator(&vma_allocator_create_info, &_vma_allocator));
    }

    void Renderer::Destroy() {
        for (VkImageView image_view: _swapchain_image_view) {
            vkDestroyImageView(_device, image_view, nullptr);
        }

        vkDeviceWaitIdle(_device);
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
        CHECK_RESULT_VK(vkCreatePipelineLayout(_device, &pipeline_layout_create_info, nullptr, &_pipeline_layout));


        // Create Graphics Pipeline

        VkShaderModule vertex_shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic.vert");
        VkShaderModule fragment_shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic.frag");

        std::array shader_stage_create_info = {
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertex_shader_module,
                .pName = "main",
                .pSpecializationInfo = nullptr
            },
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
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
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertex_input_binding_description,
            .vertexAttributeDescriptionCount = 1,
            .pVertexAttributeDescriptions = &vertex_input_attribute_description
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
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
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
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
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment_state,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };


        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
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

        // Create Buffers
        uint32_t queue_family = _device.get_queue_index(vkb::QueueType::graphics).value();
        VkBufferCreateInfo staging_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = Application::StagingBufferSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue_family
        };

        VmaAllocationCreateInfo staging_buffer_allocation_create_info{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        };

        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &staging_buffer_create_info, &staging_buffer_allocation_create_info, &_staging_buffer, &_staging_buffer_allocation, nullptr));
        CHECK_RESULT_VK(vmaMapMemory(_vma_allocator, _staging_buffer_allocation, reinterpret_cast<void**>(&_staging_buffer_pointer)));

        VmaAllocationCreateInfo index_vertex_buffer_allocation_create_info{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };

        VkBufferCreateInfo index_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = Application::IndexBufferSize,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue_family
        };

        VkBufferCreateInfo vertex_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = Application::VertexBufferSize,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue_family
        };

        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &index_buffer_create_info, &index_vertex_buffer_allocation_create_info, &_index_buffer, &_index_buffer_allocation, nullptr));
        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &vertex_buffer_create_info, &index_vertex_buffer_allocation_create_info, &_vertex_buffer, &_vertex_buffer_allocation, nullptr));

        VkSemaphoreCreateInfo semaphore_create_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        CHECK_RESULT_VK(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &_image_ready_semaphore));
        CHECK_RESULT_VK(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &_render_complete_semaphore));


        VkFenceCreateInfo fence_create_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        CHECK_RESULT_VK(vkCreateFence(_device, &fence_create_info, nullptr, &_command_buffer_fence));
    }

    void Renderer::Release() {
        vkDeviceWaitIdle(_device);

        vkDestroyFence(_device, _command_buffer_fence, nullptr);
        vkDestroySemaphore(_device, _image_ready_semaphore, nullptr);
        vkDestroySemaphore(_device, _render_complete_semaphore, nullptr);

        vmaDestroyBuffer(_vma_allocator, _vertex_buffer, _vertex_buffer_allocation);
        vmaDestroyBuffer(_vma_allocator, _index_buffer, _index_buffer_allocation);
        vmaUnmapMemory(_vma_allocator, _staging_buffer_allocation);
        _staging_buffer_pointer = nullptr;
        vmaDestroyBuffer(_vma_allocator, _staging_buffer, _staging_buffer_allocation);

        vmaDestroyAllocator(_vma_allocator);

        vkDestroyPipeline(_device, _pipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    }

    void Renderer::Upload() {
        std::ranges::copy(_model.Index, reinterpret_cast<decltype(_model.Index)::value_type*>(_staging_buffer_pointer));
        std::ranges::copy(_model.Vertex, reinterpret_cast<decltype(_model.Vertex)::value_type*>(_staging_buffer_pointer + sizeof(_model.Index)));
        CHECK_RESULT_VK(vmaFlushAllocation(_vma_allocator, _staging_buffer_allocation, 0, sizeof(_model.Index) + sizeof(_model.Vertex)));

        VkCommandBufferAllocateInfo command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = _command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer command_buffer;
        CHECK_RESULT_VK(vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, &command_buffer));

        VkCommandBufferBeginInfo command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        CHECK_RESULT_VK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

        VkBufferCopy index_copy_region{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = sizeof(_model.Index)
        };

        VkBufferCopy vertex_copy_region{
            .srcOffset = sizeof(_model.Index),
            .dstOffset = 0,
            .size = sizeof(_model.Vertex)
        };

        vkCmdCopyBuffer(command_buffer, _staging_buffer, _index_buffer, 1, &index_copy_region);
        vkCmdCopyBuffer(command_buffer, _staging_buffer, _vertex_buffer, 1, &vertex_copy_region);

        CHECK_RESULT_VK(vkEndCommandBuffer(command_buffer));

        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };

        CHECK_RESULT_VK(vkQueueSubmit(_queue, 1, &submit_info, _command_buffer_fence));

        CHECK_RESULT_VK(vkWaitForFences(_device, 1, &_command_buffer_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        CHECK_RESULT_VK(vkResetFences(_device, 1, &_command_buffer_fence));

        vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
    }


    bool Renderer::Render() {

        uint32_t queue_family = _device.get_queue_index(vkb::QueueType::graphics).value();

        uint32_t image_index;
        CHECK_RESULT_VK(vkAcquireNextImageKHR(_device, _swapchain, std::numeric_limits<uint64_t>::max(), _image_ready_semaphore, nullptr, &image_index));

        VkImage image = _swapchain_image.at(image_index);
        VkImageView image_view = _swapchain_image_view.at(image_index);

        VkCommandBufferAllocateInfo command_buffer_allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = _command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer command_buffer;
        CHECK_RESULT_VK(vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, &command_buffer));

        VkCommandBufferBeginInfo command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        CHECK_RESULT_VK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

        VkImageMemoryBarrier image_memory_barrier_before{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = queue_family,
            .dstQueueFamilyIndex = queue_family,
            .image = image,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier_before);

        VkRenderingAttachmentInfo color_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {0, 0, 0, 0}
        };

        VkRenderingInfo rendering_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = VkRect2D{
                    .offset = {0, 0},
                    .extent = _swapchain.extent
                },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr
        };

        VkDeviceSize offset_zero = 0;
        vkCmdBeginRendering(command_buffer, &rendering_info);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        vkCmdBindIndexBuffer(command_buffer, _index_buffer, offset_zero, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &_vertex_buffer, &offset_zero);
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(_model.Index.size()), 1, 0, 0, 0);
        vkCmdEndRendering(command_buffer);

        VkImageMemoryBarrier image_memory_barrier_after{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = queue_family,
            .dstQueueFamilyIndex = queue_family,
            .image = image,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier_after);

        CHECK_RESULT_VK(vkEndCommandBuffer(command_buffer));

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &_image_ready_semaphore,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &_render_complete_semaphore
        };
        CHECK_RESULT_VK(vkQueueSubmit(_queue, 1, &submit_info, _command_buffer_fence));


        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &_render_complete_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &_swapchain.swapchain,
            .pImageIndices = &image_index,
            .pResults = nullptr
        };

        CHECK_RESULT_VK(vkQueuePresentKHR(_queue, &present_info));

        CHECK_RESULT_VK(vkWaitForFences(_device, 1, &_command_buffer_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        CHECK_RESULT_VK(vkResetFences(_device, 1, &_command_buffer_fence));

        vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);

        return _window.Update();
    }


} // Vkxel