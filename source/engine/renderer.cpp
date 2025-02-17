//
// Created by jiayi on 1/18/2025.
//

#include <array>
#include <ranges>
#include <utility>
#include <vector>

#include "VkBootstrap.h"
#include "glm/glm.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "data_type.h"
#include "renderer.h"

#include "resource.h"
#include "shader.h"
#include "util/application.h"
#include "util/check.h"
#include "vkutil/buffer.h"
#include "vkutil/command.h"
#include "vkutil/image.h"


namespace Vkxel {
    Renderer::Renderer(Window &window, GUI &gui) : _window(window), _gui(gui) {}

    void Renderer::Init() {

        CHECK_NOTNULL(!_init);

        // Create Instance
        vkb::InstanceBuilder instance_builder;
        auto instance_result = instance_builder.set_app_name(Application::Name.data())
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
        _window.AddCallback(WindowEvent::Minimize, [&]() { _pause = true; });
        _window.AddCallback(WindowEvent::Restore, [&]() { _pause = false; });
        _window.AddCallback(WindowEvent::Resize, [&]() { Resize(); });

        // Select Physical Device
        VkPhysicalDeviceVulkan13Features physical_device_vulkan13_features{.synchronization2 = VK_TRUE,
                                                                           .dynamicRendering = VK_TRUE};

        vkb::PhysicalDeviceSelector physical_device_selector(_instance);
        auto physical_device_result =
                physical_device_selector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
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
        auto swapchain_result = swapchain_builder.set_old_swapchain(_swapchain)
                                        .set_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                        .set_desired_present_mode(Application::DefaultPresentMode)
                                        .build();
        CHECK_NOTNULL_MSG(swapchain_result, swapchain_result.error().message());
        _swapchain = swapchain_result.value();

        auto swapchain_image_result = _swapchain.get_images();
        CHECK_NOTNULL_MSG(swapchain_image_result, swapchain_image_result.error().message());
        _swapchain_image = std::move(swapchain_image_result.value());

        // Get Queue
        auto queue_result = _device.get_queue(vkb::QueueType::graphics);
        CHECK_NOTNULL_MSG(queue_result, queue_result.error().message());
        _queue = queue_result.value();
        _queue_family_index = _device.get_queue_index(vkb::QueueType::graphics).value();

        // Create Command Pool
        VkCommandPoolCreateInfo command_pool_create_info{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                                         .queueFamilyIndex = _queue_family_index};

        CHECK_RESULT_VK(vkCreateCommandPool(_device, &command_pool_create_info, nullptr, &_command_pool));

        // Create Descriptor Pool
        std::array descriptor_pool_size = {
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Application::DefaultDescriptorCount},
                VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, Application::DefaultDescriptorCount}};

        VkDescriptorPoolCreateInfo descriptor_pool_create_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                .maxSets = Application::DefaultDescriptorSetCount,
                .poolSizeCount = static_cast<uint32_t>(descriptor_pool_size.size()),
                .pPoolSizes = descriptor_pool_size.data()};

        CHECK_RESULT_VK(vkCreateDescriptorPool(_device, &descriptor_pool_create_info, nullptr, &_descriptor_pool));

        // Create VMA Allocator
        VmaAllocatorCreateInfo vma_allocator_create_info{
                .physicalDevice = _physical_device,
                .device = _device,
                .instance = _instance,
        };

        CHECK_RESULT_VK(vmaCreateAllocator(&vma_allocator_create_info, &_allocator));

        // Create GUI
        GuiInitInfo gui_init_info{.Instance = _instance,
                                  .PhysicalDevice = _physical_device,
                                  .Device = _device,
                                  .QueueFamily = _queue_family_index,
                                  .Queue = _queue,
                                  .DescriptorPool = _descriptor_pool,
                                  .MinImageCount = _swapchain.requested_min_image_count,
                                  .ImageCount = _swapchain.image_count,
                                  .ColorAttachmentFormat = Application::DefaultFramebufferFormat};

        _gui.InitVK(&gui_init_info);

        _init = true;
    }

    void Renderer::Destroy() {
        CHECK_NOTNULL(_init);

        vkDeviceWaitIdle(_device);

        _gui.DestroyVK();

        vmaDestroyAllocator(_allocator);
        vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
        vkDestroyCommandPool(_device, _command_pool, nullptr);
        vkb::destroy_swapchain(_swapchain);
        vkb::destroy_device(_device);
        _window.DestroySurface();
        vkb::destroy_instance(_instance);

        _init = false;
    }

    void Renderer::LoadScene(const Scene &scene) {
        CHECK_NOTNULL(_init);
        CHECK_NOTNULL(!_scene);

        _scene = scene;

        // RenderContext context;
        // _scene.value().get().Draw(context);
        // CHECK_NOTNULL_MSG(!context.objects.empty(), "Empty Scene");

        VkSemaphoreCreateInfo semaphore_create_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        CHECK_RESULT_VK(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &_image_ready_semaphore));
        CHECK_RESULT_VK(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &_render_complete_semaphore));


        VkFenceCreateInfo fence_create_info{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        CHECK_RESULT_VK(vkCreateFence(_device, &fence_create_info, nullptr, &_command_buffer_fence));


        std::array descriptor_set_layout_binding = {
                VkDescriptorSetLayoutBinding{.binding = 0,
                                             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                             .descriptorCount = 1,
                                             .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}};

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(descriptor_set_layout_binding.size()),
                .pBindings = descriptor_set_layout_binding.data()};

        CHECK_RESULT_VK(vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_create_info, nullptr,
                                                    &_descriptor_set_layout_frame));
        CHECK_RESULT_VK(vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_create_info, nullptr,
                                                    &_descriptor_set_layout_object));


        // Upload Data
        _resource_manager = std::make_unique<ResourceManager>(_device, _queue_family_index, _queue, _command_pool,
                                                              _descriptor_pool, _descriptor_set_layout_frame,
                                                              _descriptor_set_layout_object, _allocator);
        _resource_uploader =
                std::make_unique<ResourceUploader>(_device, _queue_family_index, _queue, _command_pool, _allocator);

        _frame_resource = _resource_manager->CreateFrameResource(_swapchain.extent.width, _swapchain.extent.height);

        // _object_resource.reserve(context.objects.size());
        //
        // for (const auto &object: context.objects) {
        //     ObjectResource &resource =
        //     _object_resource.emplace_back(_resource_manager->CreateObjectResource(object));
        //     _resource_uploader->AddObject(object, resource);
        // }
        //
        // _resource_uploader->Upload();


        // Create Pipeline Layout

        std::array pipeline_descriptor_set_layouts = {_descriptor_set_layout_frame, _descriptor_set_layout_object};

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = static_cast<uint32_t>(pipeline_descriptor_set_layouts.size()),
                .pSetLayouts = pipeline_descriptor_set_layouts.data(),
                .pushConstantRangeCount = 0};
        CHECK_RESULT_VK(vkCreatePipelineLayout(_device, &pipeline_layout_create_info, nullptr, &_pipeline_layout));


        // Create Graphics Pipeline

        VkShaderModule shader_module = ShaderLoader::Instance().LoadToModule(_device, "basic");

        std::array shader_stage_create_info = {
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
        };

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

        VkPipelineVertexInputStateCreateInfo input_state_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &vertex_input_binding_description,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attribute_description.size()),
                .pVertexAttributeDescriptions = vertex_input_attribute_description.data()};

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE};

        VkPipelineViewportStateCreateInfo viewport_state_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1,
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
                .lineWidth = 1.0f};

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE};

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
                .maxDepthBounds = 1.0f};

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

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &color_blend_attachment_state,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};


        std::array dynamic_state = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = static_cast<uint32_t>(dynamic_state.size()),
                .pDynamicStates = dynamic_state.data()};

        VkPipelineRenderingCreateInfo pipeline_rendering_create_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &_frame_resource.colorImage.createInfo.format,
                .depthAttachmentFormat = _frame_resource.depthImage.createInfo.format};

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
                .basePipelineIndex = -1};


        CHECK_RESULT_VK(
                vkCreateGraphicsPipelines(_device, nullptr, 1, &graphics_pipeline_create_info, nullptr, &_pipeline));

        vkDestroyShaderModule(_device, shader_module, nullptr);
    }

    void Renderer::UnloadScene() {
        CHECK_NOTNULL(_init);
        CHECK_NOTNULL(_scene);

        vkDeviceWaitIdle(_device);

        for (auto &object: _object_resource | std::views::values) {
            _resource_manager->DestroyObjectResource(object);
        }
        _object_resource.clear();

        _resource_manager->DestroyFrameResource(_frame_resource);

        vkDestroyFence(_device, _command_buffer_fence, nullptr);
        vkDestroySemaphore(_device, _image_ready_semaphore, nullptr);
        vkDestroySemaphore(_device, _render_complete_semaphore, nullptr);

        vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout_frame, nullptr);
        vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout_object, nullptr);


        vkDestroyPipeline(_device, _pipeline, nullptr);
        vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);

        _scene = std::nullopt;
    }

    void Renderer::Render() {

        CHECK_NOTNULL(_init);
        CHECK_NOTNULL(_scene);

        if (_pause) {
            return;
        }

        // Release Outdated Object Resource
        for (auto it = _object_resource.begin(); it != _object_resource.end();) {
            if (auto &[object_id, object_resource] = *it; !object_resource.isActive) {
                _resource_manager->DestroyObjectResource(object_resource);
                it = _object_resource.erase(it);
            } else {
                // Mark Object As Inactive
                object_resource.isActive = false;
                ++it;
            }
        }

        RenderContext context;
        _scene.value().get().Draw(context);

        uint32_t image_index;
        CHECK_RESULT_VK(vkAcquireNextImageKHR(_device, _swapchain, std::numeric_limits<uint64_t>::max(),
                                              _image_ready_semaphore, nullptr, &image_index));

        VkImage present_image = _swapchain_image.at(image_index);

        VkCommandBufferAllocateInfo command_buffer_allocate_info{.sType =
                                                                         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                                 .commandPool = _command_pool,
                                                                 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                 .commandBufferCount = 1};

        VkCommandBuffer command_buffer;
        CHECK_RESULT_VK(vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, &command_buffer));

        VkCommandBufferBeginInfo command_buffer_begin_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                           .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

        CHECK_RESULT_VK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

        VkViewport viewport{.x = 0.0f,
                            .y = 0.0f,
                            .width = static_cast<float>(_swapchain.extent.width),
                            .height = static_cast<float>(_swapchain.extent.height),
                            .minDepth = 0.0f,
                            .maxDepth = 1.0f};
        VkRect2D scissor{.offset = VkOffset2D{0, 0}, .extent = _swapchain.extent};

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Upload Scene
        for (const auto &object: context.objects) {
            if (_object_resource.contains(object.objectId)) {
                ObjectResource &object_resource = _object_resource.at(object.objectId);
                if (object.isDirty) {
                    _resource_manager->DestroyObjectResource(object_resource);
                    object_resource = _resource_manager->CreateObjectResource(object);
                    _resource_uploader->AddObject(object, object_resource);
                } else {
                    object_resource.isActive = true;
                }
                _resource_manager->UpdateObjectResource(command_buffer, object, object_resource);
            } else {
                ObjectResource &object_resource = _object_resource[object.objectId] =
                        _resource_manager->CreateObjectResource(object);
                _resource_uploader->AddObject(object, object_resource);
                _resource_manager->UpdateObjectResource(command_buffer, object, object_resource);
            }
        }
        _resource_manager->UpdateFrameResource(command_buffer, context.scene, _frame_resource);

        // Upload Object Mesh Data (Block Wait)
        // TODO Support Async Upload
        _resource_uploader->Upload();

        _frame_resource.colorImage.CmdBarrier(command_buffer, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


        // Color Pass
        VkRenderingAttachmentInfo color_attachment_info{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                                        .imageView = _frame_resource.colorImage.imageView,
                                                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                                                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                        .clearValue = {0, 0, 0, 0}};

        VkRenderingAttachmentInfo depth_attachment_info{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                                        .imageView = _frame_resource.depthImage.imageView,
                                                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                                                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                        .clearValue = {.depthStencil = {.depth = 1}}};

        VkRenderingInfo rendering_info{.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                                       .renderArea = VkRect2D{.offset = {0, 0}, .extent = _swapchain.extent},
                                       .layerCount = 1,
                                       .colorAttachmentCount = 1,
                                       .pColorAttachments = &color_attachment_info,
                                       .pDepthAttachment = &depth_attachment_info,
                                       .pStencilAttachment = nullptr};

        VkDeviceSize offset_zero = 0;
        vkCmdBeginRendering(command_buffer, &rendering_info); // Camera Pass
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1,
                                &_frame_resource.descriptorSet.set, 0, nullptr);

        for (const auto &object: _object_resource | std::views::values) {
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 1, 1,
                                    &object.descriptorSet.set, 0, nullptr);
            vkCmdBindIndexBuffer(command_buffer, object.indexBuffer.buffer, offset_zero, VK_INDEX_TYPE_UINT32);
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &object.vertexBuffer.buffer, &offset_zero);
            vkCmdDrawIndexed(command_buffer, object.indexCount, 1, object.firstIndex, 0, 0);
        }

        vkCmdEndRendering(command_buffer);

        // UI Pass
        _frame_resource.colorImage.CmdBarrier(
                command_buffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

        VkRenderingAttachmentInfo color_attachment_info_ui{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                                           .imageView = _frame_resource.colorImage.imageView,
                                                           .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                                                           .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                                           .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                           .clearValue = {0, 0, 0, 0}};

        VkRenderingInfo rendering_info_ui{.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                                          .renderArea = VkRect2D{.offset = {0, 0}, .extent = _swapchain.extent},
                                          .layerCount = 1,
                                          .colorAttachmentCount = 1,
                                          .pColorAttachments = &color_attachment_info_ui,
                                          .pDepthAttachment = nullptr,
                                          .pStencilAttachment = nullptr};

        vkCmdBeginRendering(command_buffer, &rendering_info_ui);
        _gui.Render(command_buffer);
        vkCmdEndRendering(command_buffer);

        _frame_resource.colorImage.CmdBarrier(
                command_buffer, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        VkImageMemoryBarrier2 present_image_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = present_image,
                .subresourceRange = VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                            .baseMipLevel = 0,
                                                            .levelCount = 1,
                                                            .baseArrayLayer = 0,
                                                            .layerCount = 1}};

        VkDependencyInfo dependency_info{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                         .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                                         .imageMemoryBarrierCount = 1,
                                         .pImageMemoryBarriers = &present_image_memory_barrier};

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);

        // Perform Blit Operation
        VkImageBlit2 blit_region{
                .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,

                .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .mipLevel = 0,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1},
                .srcOffsets = {{0, 0, 0},
                               {static_cast<int32_t>(_swapchain.extent.width),
                                static_cast<int32_t>(_swapchain.extent.height), 1}},

                .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .mipLevel = 0,
                                   .baseArrayLayer = 0,
                                   .layerCount = 1},
                .dstOffsets = {{0, 0, 0},
                               {static_cast<int32_t>(_swapchain.extent.width),
                                static_cast<int32_t>(_swapchain.extent.height), 1}},
        };

        VkBlitImageInfo2 blit_info{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                                   .srcImage = _frame_resource.colorImage.image,
                                   .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   .dstImage = present_image,
                                   .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   .regionCount = 1,
                                   .pRegions = &blit_region,
                                   .filter = VK_FILTER_LINEAR};

        vkCmdBlitImage2(command_buffer, &blit_info);

        present_image_memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
        present_image_memory_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        present_image_memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        present_image_memory_barrier.dstAccessMask = VK_ACCESS_2_NONE_KHR;
        present_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        present_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);

        CHECK_RESULT_VK(vkEndCommandBuffer(command_buffer));

        VkCommandBufferSubmitInfo command_buffer_submit_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                                                             .commandBuffer = command_buffer};
        VkSemaphoreSubmitInfo image_ready_semaphore_submit_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                                                .semaphore = _image_ready_semaphore,
                                                                .stageMask = VK_PIPELINE_STAGE_2_BLIT_BIT};

        VkSemaphoreSubmitInfo render_complete_semaphore_submit_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                                                    .semaphore = _render_complete_semaphore,
                                                                    .stageMask =
                                                                            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT};

        VkSubmitInfo2 submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                                  .waitSemaphoreInfoCount = 1,
                                  .pWaitSemaphoreInfos = &image_ready_semaphore_submit_info,
                                  .commandBufferInfoCount = 1,
                                  .pCommandBufferInfos = &command_buffer_submit_info,
                                  .signalSemaphoreInfoCount = 1,
                                  .pSignalSemaphoreInfos = &render_complete_semaphore_submit_info};
        vkQueueSubmit2(_queue, 1, &submit_info, _command_buffer_fence);


        VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                      .waitSemaphoreCount = 1,
                                      .pWaitSemaphores = &_render_complete_semaphore,
                                      .swapchainCount = 1,
                                      .pSwapchains = &_swapchain.swapchain,
                                      .pImageIndices = &image_index,
                                      .pResults = nullptr};

        CHECK_RESULT_VK(vkQueuePresentKHR(_queue, &present_info));

        CHECK_RESULT_VK(
                vkWaitForFences(_device, 1, &_command_buffer_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        CHECK_RESULT_VK(vkResetFences(_device, 1, &_command_buffer_fence));

        vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
    }

    void Renderer::Resize() {
        vkDeviceWaitIdle(_device);

        vkb::SwapchainBuilder swapchain_builder(_device);
        auto swapchain_result = swapchain_builder.set_old_swapchain(_swapchain)
                                        .set_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                        .set_desired_present_mode(Application::DefaultPresentMode)
                                        .build();
        CHECK_NOTNULL_MSG(swapchain_result, swapchain_result.error().message());
        vkb::destroy_swapchain(_swapchain);
        _swapchain = swapchain_result.value();

        auto swapchain_image_result = _swapchain.get_images();
        CHECK_NOTNULL_MSG(swapchain_image_result, swapchain_image_result.error().message());
        _swapchain_image = std::move(swapchain_image_result.value());


        _resource_manager->DestroyFrameResource(_frame_resource);
        _frame_resource = _resource_manager->CreateFrameResource(_swapchain.extent.width, _swapchain.extent.height);
    }


    Window &Renderer::GetWindow() const { return _window; }

    GUI &Renderer::GetGUI() const { return _gui; }


} // namespace Vkxel
