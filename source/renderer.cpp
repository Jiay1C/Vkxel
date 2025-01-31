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
#include "buffer.h"


namespace Vkxel {
     Renderer::Renderer(Window& window, Camera& camera, GUI& gui) : _window(window), _camera(camera), _gui(gui) { }

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
        _surface = _window.CreateSurface(_instance);

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
        _queue_family_index = _device.get_queue_index(vkb::QueueType::graphics).value();

        // Create Command Pool
        VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = _queue_family_index
        };

        CHECK_RESULT_VK(vkCreateCommandPool(_device, &command_pool_create_info, nullptr, &_command_pool));

        // Create Descriptor Pool
        std::array descriptor_pool_size = {
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Application::DefaultDescriptorCount },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, Application::DefaultDescriptorCount }
        };

        VkDescriptorPoolCreateInfo descriptor_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = Application::DefaultDescriptorSetCount,
            .poolSizeCount = static_cast<uint32_t>(descriptor_pool_size.size()),
            .pPoolSizes = descriptor_pool_size.data()
        };

        CHECK_RESULT_VK(vkCreateDescriptorPool(_device, &descriptor_pool_create_info, nullptr, &_descriptor_pool));

        // Create VMA Allocator
        VmaAllocatorCreateInfo vma_allocator_create_info{
            .physicalDevice = _physical_device,
            .device = _device,
            .instance = _instance,
        };

        CHECK_RESULT_VK(vmaCreateAllocator(&vma_allocator_create_info, &_vma_allocator));

        // Create GUI
        GuiInitInfo gui_init_info{
            .Instance = _instance,
            .PhysicalDevice = _physical_device,
            .Device = _device,
            .QueueFamily = _queue_family_index,
            .Queue = _queue,
            .DescriptorPool = _descriptor_pool,
            .MinImageCount = _swapchain.requested_min_image_count,
            .ImageCount = _swapchain.image_count,
            .ColorAttachmentFormat = _swapchain.image_format
        };

        _gui.InitVK(&gui_init_info);
    }

    void Renderer::Destroy() {
        vkDeviceWaitIdle(_device);

        _gui.DestroyVK();

        for (VkImageView image_view: _swapchain_image_view) {
            vkDestroyImageView(_device, image_view, nullptr);
        }

        vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
        vkDestroyCommandPool(_device, _command_pool, nullptr);
        vkb::destroy_swapchain(_swapchain);
        vkb::destroy_device(_device);
        _window.DestroySurface();
        vkb::destroy_instance(_instance);
    }

    void Renderer::AllocateResource() {
        // Create Buffers
        VkBufferCreateInfo staging_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = Application::DefaultStagingBufferSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &_queue_family_index
        };

        VmaAllocationCreateInfo staging_buffer_allocation_create_info{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        };

        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &staging_buffer_create_info, &staging_buffer_allocation_create_info, &_staging_buffer, &_staging_buffer_allocation, nullptr));
        CHECK_RESULT_VK(vmaMapMemory(_vma_allocator, _staging_buffer_allocation, reinterpret_cast<void**>(&_staging_buffer_pointer)));

        VmaAllocationCreateInfo buffer_allocation_create_info{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };

        VkBufferCreateInfo index_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = Application::DefaultIndexBufferSize,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &_queue_family_index
        };

        VkBufferCreateInfo vertex_buffer_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = Application::DefaultVertexBufferSize,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &_queue_family_index
        };

        VkBufferCreateInfo constant_buffer_per_frame_create_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(ConstantBufferPerFrame),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &_queue_family_index
        };

        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &index_buffer_create_info, &buffer_allocation_create_info, &_index_buffer, &_index_buffer_allocation, nullptr));
        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &vertex_buffer_create_info, &buffer_allocation_create_info, &_vertex_buffer, &_vertex_buffer_allocation, nullptr));
        CHECK_RESULT_VK(vmaCreateBuffer(_vma_allocator, &constant_buffer_per_frame_create_info, &buffer_allocation_create_info, &_constant_buffer_per_frame, &_constant_buffer_per_frame_allocation, nullptr));

        VkImageCreateInfo depth_image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_D32_SFLOAT,
            .extent = VkExtent3D{
                .width = _swapchain.extent.width,
                .height = _swapchain.extent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &_queue_family_index,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo depth_image_allocation_create_info{
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };
        CHECK_RESULT_VK(vmaCreateImage(_vma_allocator, &depth_image_create_info, &depth_image_allocation_create_info, &_depth_image, &_depth_image_allocation, nullptr));

        VkImageViewCreateInfo depth_image_view_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = _depth_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_D32_SFLOAT,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        CHECK_RESULT_VK(vkCreateImageView(_device, &depth_image_view_create_info, nullptr, &_depth_image_view));


        std::array descriptor_set_layout_binding = {
            VkDescriptorSetLayoutBinding{
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            }
        };

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(descriptor_set_layout_binding.size()),
            .pBindings = descriptor_set_layout_binding.data()
        };

        CHECK_RESULT_VK(vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_create_info, nullptr, &_descriptor_set_layout));



        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = _descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &_descriptor_set_layout
        };

        CHECK_RESULT_VK(vkAllocateDescriptorSets(_device, &descriptor_set_allocate_info, &_descriptor_set));

        VkDescriptorBufferInfo constant_buffer_per_frame_info{
            .buffer = _constant_buffer_per_frame,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        };
        std::array descriptor_set_write_info = {
            VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _descriptor_set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &constant_buffer_per_frame_info,
            }
        };

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptor_set_write_info.size()), descriptor_set_write_info.data(), 0, nullptr);

        // Create Pipeline Layout
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &_descriptor_set_layout,
            .pushConstantRangeCount = 0,
            // TODO: Add Descriptor Set
        };
        CHECK_RESULT_VK(vkCreatePipelineLayout(_device, &pipeline_layout_create_info, nullptr, &_pipeline_layout));


        // Create Graphics Pipeline

        VkShaderModule shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic");

        std::array shader_stage_create_info = {
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = shader_module,
                .pName = "vertexMain",
                .pSpecializationInfo = nullptr
            },
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = shader_module,
                .pName = "fragmentMain",
                .pSpecializationInfo = nullptr
            },
        };

        VkVertexInputBindingDescription vertex_input_binding_description{
            .binding = 0,
            .stride = sizeof(VertexInput),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        std::array vertex_input_attribute_description = {
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(VertexInput, position)
            },
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(VertexInput, normal)
            },
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(VertexInput, color)
            }
        };

        VkPipelineVertexInputStateCreateInfo input_state_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertex_input_binding_description,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attribute_description.size()),
            .pVertexAttributeDescriptions = vertex_input_attribute_description.data()
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
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
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
            .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT
        };

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipeline_rendering_create_info,
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

        vkDestroyShaderModule(_device, shader_module, nullptr);

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

    void Renderer::ReleaseResource() {
        vkDeviceWaitIdle(_device);

        vkDestroyFence(_device, _command_buffer_fence, nullptr);
        vkDestroySemaphore(_device, _image_ready_semaphore, nullptr);
        vkDestroySemaphore(_device, _render_complete_semaphore, nullptr);

        vkFreeDescriptorSets(_device, _descriptor_pool, 1, &_descriptor_set);
        vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);

        vkDestroyImageView(_device, _depth_image_view, nullptr);
        vmaDestroyImage(_vma_allocator, _depth_image, _depth_image_allocation);

        vmaDestroyBuffer(_vma_allocator, _constant_buffer_per_frame, _constant_buffer_per_frame_allocation);
        vmaDestroyBuffer(_vma_allocator, _vertex_buffer, _vertex_buffer_allocation);
        vmaDestroyBuffer(_vma_allocator, _index_buffer, _index_buffer_allocation);
        vmaUnmapMemory(_vma_allocator, _staging_buffer_allocation);
        _staging_buffer_pointer = nullptr;
        vmaDestroyBuffer(_vma_allocator, _staging_buffer, _staging_buffer_allocation);

        vmaDestroyAllocator(_vma_allocator);

        vkDestroyPipeline(_device, _pipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    }

    void Renderer::UploadData() {
        uint32_t index_buffer_size = static_cast<uint32_t>(_model.index.size() * sizeof(decltype(_model.index)::value_type));
        uint32_t vertex_buffer_size = static_cast<uint32_t>(_model.vertex.size() * sizeof(decltype(_model.vertex)::value_type));
        std::ranges::copy(_model.index, reinterpret_cast<decltype(_model.index)::value_type*>(_staging_buffer_pointer));
        std::ranges::copy(_model.vertex, reinterpret_cast<decltype(_model.vertex)::value_type*>(_staging_buffer_pointer + index_buffer_size));
        CHECK_RESULT_VK(vmaFlushAllocation(_vma_allocator, _staging_buffer_allocation, 0, index_buffer_size + vertex_buffer_size));

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
            .size = index_buffer_size
        };

        VkBufferCopy vertex_copy_region{
            .srcOffset = index_buffer_size,
            .dstOffset = 0,
            .size = vertex_buffer_size
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


    void Renderer::Render() {

        // Create Constant Buffer Per Frame
        ConstantBufferPerFrame constant_buffer_per_frame{
            .ModelMatrix = glm::transpose(_model.transform.GetLocalToWorldMatrix()),
            .ViewMatrix = glm::transpose(_camera.GetViewMatrix()),
            .ProjectionMatrix = glm::transpose(_camera.GetProjectionMatrix())
        };

        uint32_t image_index;
        CHECK_RESULT_VK(vkAcquireNextImageKHR(_device, _swapchain, std::numeric_limits<uint64_t>::max(), _image_ready_semaphore, nullptr, &image_index));

        VkImage color_attachment_image = _swapchain_image.at(image_index);
        VkImageView color_attachment_image_view = _swapchain_image_view.at(image_index);

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

        vkCmdUpdateBuffer(command_buffer, _constant_buffer_per_frame, 0, sizeof(ConstantBufferPerFrame), &constant_buffer_per_frame);

        VkImageMemoryBarrier image_memory_barrier_before{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = _queue_family_index,
            .dstQueueFamilyIndex = _queue_family_index,
            .image = color_attachment_image,
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
            .imageView = color_attachment_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {0, 0, 0, 0}
        };

        VkRenderingAttachmentInfo depth_attachment_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = _depth_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {.depthStencil = {.depth = 1}}
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
            .pDepthAttachment = &depth_attachment_info,
            .pStencilAttachment = nullptr
        };

        VkDeviceSize offset_zero = 0;
        vkCmdBeginRendering(command_buffer, &rendering_info); // Camera Pass
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &_descriptor_set, 0, nullptr);
        vkCmdBindIndexBuffer(command_buffer, _index_buffer, offset_zero, VK_INDEX_TYPE_UINT32);
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &_vertex_buffer, &offset_zero);
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(_model.index.size()), 1, 0, 0, 0);
        vkCmdEndRendering(command_buffer);

        VkImageMemoryBarrier image_memory_barrier_ui{
         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
         .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
         .srcQueueFamilyIndex = _queue_family_index,
         .dstQueueFamilyIndex = _queue_family_index,
         .image = color_attachment_image,
         .subresourceRange = VkImageSubresourceRange{
             .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
             .baseMipLevel = 0,
             .levelCount = 1,
             .baseArrayLayer = 0,
             .layerCount = 1
         }
        };

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
         0, nullptr,
         0, nullptr,
         1, &image_memory_barrier_ui);

        VkRenderingAttachmentInfo color_attachment_info_ui{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = color_attachment_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {0, 0, 0, 0}
        };

        VkRenderingInfo rendering_info_ui{
             .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
             .renderArea = VkRect2D{
                 .offset = {0, 0},
                 .extent = _swapchain.extent
             },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_info_ui,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr
        };

        vkCmdBeginRendering(command_buffer, &rendering_info_ui); // UI Pass
        _gui.Render(command_buffer);
        vkCmdEndRendering(command_buffer);

        VkImageMemoryBarrier image_memory_barrier_after{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = _queue_family_index,
            .dstQueueFamilyIndex = _queue_family_index,
            .image = color_attachment_image,
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
    }

    Camera &Renderer::GetCamera() const {
        return _camera;
    }

    Window &Renderer::GetWindow() const {
        return _window;
    }

} // Vkxel