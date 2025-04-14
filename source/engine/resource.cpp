//
// Created by jiayi on 2/7/2025.
//

#include <array>
#include <list>
#include <ranges>
#include <vector>

#include "resource.h"

#include "util/application.h"
#include "util/check.h"
#include "vkutil/command.h"
#include "vtime.h"

namespace Vkxel {
    ResourceUploader &ResourceUploader::AddObject(const ObjectData &object, ObjectResource &resource) {
        _objects.emplace_back(object, resource);
        return *this;
    }

    void ResourceUploader::UploadObjects() {
        if (_objects.empty()) {
            return;
        }

        VkDeviceSize total_size = 0;

        for (const auto &object_wrapper: _objects | std::views::keys) {
            if (const ObjectData &object = object_wrapper; std::holds_alternative<CPUMeshData>(object.mesh)) {
                const auto &[index, vertex] = std::get<CPUMeshData>(object.mesh);
                total_size += sizeof(IndexType) * index.size();
                total_size += sizeof(VertexType) * vertex.size();
                // total_size += sizeof(ConstantBufferPerObject);
            }
        }

        if (total_size > 0) {
            VkUtil::Buffer staging_buffer =
                    VkUtil::BufferBuilder(_device, _allocator)
                            .SetSize(total_size)
                            .SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .SetPQueueFamilyIndices(&_queue_family)
                            .SetAllocationFlags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
                            .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
                            .SetRequiredFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                            .Build();
            staging_buffer.Create();

            std::byte *host_buffer = staging_buffer.Map();
            VkDeviceSize host_buffer_offset = 0;

            VkUtil::ImmediateCommand immediate_command(_device, _queue, _command_pool);
            VkCommandBuffer command_buffer = immediate_command.Begin();

            for (const auto &[object_wrapper, resource_wrapper]: _objects) {
                const ObjectData &object = object_wrapper;
                const ObjectResource &resource = resource_wrapper;

                if (std::holds_alternative<CPUMeshData>(object.mesh)) {
                    const auto &[index, vertex] = std::get<CPUMeshData>(object.mesh);

                    // Upload Index Buffer
                    std::ranges::copy(index, reinterpret_cast<IndexType *>(host_buffer + host_buffer_offset));
                    VkBufferCopy index_copy_region{.srcOffset = host_buffer_offset,
                                                   .dstOffset = 0,
                                                   .size = resource.indexBuffer.createInfo.size};
                    vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, resource.indexBuffer.buffer, 1,
                                    &index_copy_region);
                    host_buffer_offset += static_cast<uint32_t>(resource.indexBuffer.createInfo.size);

                    // Upload Vertex Buffer
                    std::ranges::copy(vertex, reinterpret_cast<VertexType *>(host_buffer + host_buffer_offset));
                    VkBufferCopy vertex_copy_region{.srcOffset = host_buffer_offset,
                                                    .dstOffset = 0,
                                                    .size = resource.vertexBuffer.createInfo.size};
                    vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, resource.vertexBuffer.buffer, 1,
                                    &vertex_copy_region);
                    host_buffer_offset += static_cast<uint32_t>(resource.vertexBuffer.createInfo.size);
                }

                // Change Constant Buffer Upload To Per Frame

                // Upload Constant Buffer
                // Change Matrix To Row Major
                // ConstantBufferPerObject constant_buffer_per_object{.transformMatrix =
                // glm::transpose(object.transform)}; *reinterpret_cast<ConstantBufferPerObject *>(host_buffer +
                // host_buffer_offset) = constant_buffer_per_object;
                //
                // VkBufferCopy constant_copy_region{
                //         .srcOffset = host_buffer_offset, .dstOffset = 0, .size =
                //         resource.constantBuffer.createInfo.size};
                // vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, resource.constantBuffer.buffer, 1,
                //                 &constant_copy_region);
                // host_buffer_offset += static_cast<uint32_t>(resource.constantBuffer.createInfo.size);
            }

            staging_buffer.Flush();

            immediate_command.End();

            staging_buffer.Unmap();
            staging_buffer.Destroy();
        }

        VkUtil::ImmediateCommand immediate_command(_device, _queue, _command_pool);
        VkCommandBuffer command_buffer = immediate_command.Begin();

        for (const auto &[object_wrapper, resource_wrapper]: _objects) {
            const ObjectData &object = object_wrapper;
            const ObjectResource &resource = resource_wrapper;

            if (std::holds_alternative<GPUMeshData>(object.mesh)) {
                const auto &[index_count, vertex_count, index, vertex] = std::get<GPUMeshData>(object.mesh);

                VkBufferCopy index_copy_region{.srcOffset = 0, .dstOffset = 0, .size = index_count * sizeof(IndexType)};
                vkCmdCopyBuffer(command_buffer, index.buffer, resource.indexBuffer.buffer, 1, &index_copy_region);

                VkBufferCopy vertex_copy_region{
                        .srcOffset = 0, .dstOffset = 0, .size = vertex_count * sizeof(VertexType)};
                vkCmdCopyBuffer(command_buffer, vertex.buffer, resource.vertexBuffer.buffer, 1, &vertex_copy_region);
            }
        }

        immediate_command.End();

        _objects.clear();
    }

    void ResourceUploader::Upload() { UploadObjects(); }


    ObjectResource ResourceManager::CreateObjectResource(const ObjectData &object) {

        ObjectResource resource = {.isActive = true, .firstIndex = 0};

        VkUtil::BufferBuilder buffer_builder(_device, _allocator);
        buffer_builder.SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE).SetPQueueFamilyIndices(&_queue_family);

        if (std::holds_alternative<GPUMeshData>(object.mesh)) {
            const auto &[index_count, vertex_count, index, vertex] = std::get<GPUMeshData>(object.mesh);

            resource.indexBuffer =
                    VkUtil::BufferBuilder(index)
                            .SetSize(index_count * sizeof(IndexType))
                            .SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .Build();
            resource.indexBuffer.Create();

            resource.vertexBuffer =
                    VkUtil::BufferBuilder(vertex)
                            .SetSize(vertex_count * sizeof(VertexType))
                            .SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .Build();
            resource.vertexBuffer.Create();

            resource.indexCount = index_count;
        } else if (std::holds_alternative<CPUMeshData>(object.mesh)) {
            const auto &[index, vertex] = std::get<CPUMeshData>(object.mesh);

            // Create Index Buffer
            resource.indexBuffer =
                    buffer_builder.SetSize(sizeof(IndexType) * index.size())
                            .SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .Build();
            resource.indexBuffer.Create();

            // Create Vertex Buffer
            resource.vertexBuffer =
                    buffer_builder.SetSize(sizeof(VertexType) * vertex.size())
                            .SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .Build();
            resource.vertexBuffer.Create();

            resource.indexCount = index.size();
        } else {
            CHECK(nullptr, "Error Mesh Type");
        }

        // Create Constant Buffer
        // Change Matrix To Row Major
        resource.constantBuffer =
                buffer_builder.SetSize(sizeof(ConstantBufferPerObject))
                        .SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .Build();
        resource.constantBuffer.Create();

        // Create DescriptorSet
        resource.descriptorSet =
                VkUtil::DescriptorSetBuilder(_device, _descriptor_pool, _descriptor_set_layout_object).Build();
        resource.descriptorSet.Create();

        VkDescriptorBufferInfo constant_buffer_per_object_info{
                .buffer = resource.constantBuffer.buffer, .offset = 0, .range = VK_WHOLE_SIZE};
        std::array descriptor_set_write_info = {VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = resource.descriptorSet.set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &constant_buffer_per_object_info,
        }};

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptor_set_write_info.size()),
                               descriptor_set_write_info.data(), 0, nullptr);

        return resource;
    }

    void ResourceManager::UpdateObjectResource(const VkCommandBuffer commandBuffer, const ObjectData &object,
                                               ObjectResource &objectResource) {
        objectResource.constantBuffer.CmdBarrier(commandBuffer, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                                                 VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

        // Update Constant Buffer
        // Change Matrix To Row Major
        ConstantBufferPerObject constant_buffer_per_object = {.transformMatrix = glm::transpose(object.transform)};
        vkCmdUpdateBuffer(commandBuffer, objectResource.constantBuffer.buffer, 0, sizeof(ConstantBufferPerObject),
                          &constant_buffer_per_object);

        objectResource.constantBuffer.CmdBarrier(
                commandBuffer, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT);
    }


    void ResourceManager::DestroyObjectResource(ObjectResource &resource) {
        resource.indexBuffer.Destroy();
        resource.vertexBuffer.Destroy();
        resource.constantBuffer.Destroy();
        resource.descriptorSet.Destroy();

        resource = {};
    }

    FrameResource ResourceManager::CreateFrameResource(uint32_t swapchainWidth, uint32_t swapchainHeight) {
        VkUtil::Image depth_image = VkUtil::ImageBuilder(_device, _allocator)
                                            .SetImageType(VK_IMAGE_TYPE_2D)
                                            .SetFormat(Application::DefaultDepthFormat)
                                            .SetExtent({swapchainWidth, swapchainHeight, 1})
                                            .SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                            .SetPQueueFamilyIndices(&_queue_family)
                                            .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                                            .SetViewSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                                      .baseMipLevel = 0,
                                                                      .levelCount = 1,
                                                                      .baseArrayLayer = 0,
                                                                      .layerCount = 1})
                                            .CreateImageView()
                                            .Build();
        depth_image.Create();

        VkUtil::Image color_image =
                VkUtil::ImageBuilder(_device, _allocator)
                        .SetImageType(VK_IMAGE_TYPE_2D)
                        .SetFormat(Application::DefaultColorFormat)
                        .SetExtent({swapchainWidth, swapchainHeight, 1})
                        .SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                        .SetPQueueFamilyIndices(&_queue_family)
                        .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                        .SetViewSubresourceRange({.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                  .baseMipLevel = 0,
                                                  .levelCount = 1,
                                                  .baseArrayLayer = 0,
                                                  .layerCount = 1})
                        .CreateImageView()
                        .Build();
        color_image.Create();

        VkUtil::Buffer constant_buffer =
                VkUtil::BufferBuilder(_device, _allocator)
                        .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
                        .SetPQueueFamilyIndices(&_queue_family)
                        .SetSize(sizeof(ConstantBufferPerFrame))
                        .SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .Build();
        constant_buffer.Create();

        // Create Scene DescriptorSet
        VkUtil::DescriptorSet descriptor_set =
                VkUtil::DescriptorSetBuilder(_device, _descriptor_pool, _descriptor_set_layout_frame).Build();
        descriptor_set.Create();

        VkDescriptorBufferInfo constant_buffer_per_frame_info{
                .buffer = constant_buffer.buffer, .offset = 0, .range = VK_WHOLE_SIZE};
        std::array descriptor_set_write_info = {VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor_set.set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &constant_buffer_per_frame_info,
        }};

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptor_set_write_info.size()),
                               descriptor_set_write_info.data(), 0, nullptr);

        VkSemaphore image_ready_semaphore = nullptr;
        VkSemaphore render_complete_semaphore = nullptr;
        VkSemaphoreCreateInfo semaphore_create_info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        CHECK_RESULT_VK(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &image_ready_semaphore));
        CHECK_RESULT_VK(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &render_complete_semaphore));

        VkCommandBuffer command_buffer = nullptr;
        VkCommandBufferAllocateInfo command_buffer_allocate_info{.sType =
                                                                         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                                 .commandPool = _command_pool,
                                                                 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                 .commandBufferCount = 1};

        CHECK_RESULT_VK(vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, &command_buffer));

        VkFence command_fence = nullptr;
        VkFenceCreateInfo fence_create_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                            .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        CHECK_RESULT_VK(vkCreateFence(_device, &fence_create_info, nullptr, &command_fence));

        return {.constantBuffer = constant_buffer,
                .colorImage = color_image,
                .depthImage = depth_image,
                .descriptorSet = descriptor_set,
                .imageReadySemaphore = image_ready_semaphore,
                .renderCompleteSemaphore = render_complete_semaphore,
                .commandBuffer = command_buffer,
                .commandFence = command_fence};
    }

    void ResourceManager::UpdateFrameResource(const VkCommandBuffer commandBuffer, const SceneData &scene,
                                              FrameResource &frameResource) {
        frameResource.constantBuffer.CmdBarrier(commandBuffer, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                                                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

        // Update Constant Buffer
        // Change Matrix To Row Major
        ConstantBufferPerFrame constant_buffer_per_frame{
                .frame = {.tick = static_cast<uint32_t>(Time::GetTicks()), .time = Time::GetSeconds()},
                .scene = {.viewMatrix = glm::transpose(scene.viewMatrix),
                          .projectionMatrix = glm::transpose(scene.projectionMatrix),
                          .cameraPosition = scene.cameraPosition}};
        vkCmdUpdateBuffer(commandBuffer, frameResource.constantBuffer.buffer, 0, sizeof(ConstantBufferPerFrame),
                          &constant_buffer_per_frame);

        frameResource.constantBuffer.CmdBarrier(
                commandBuffer, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT);
    }


    void ResourceManager::DestroyFrameResource(FrameResource &resource) {
        resource.constantBuffer.Destroy();
        resource.depthImage.Destroy();
        resource.colorImage.Destroy();
        resource.descriptorSet.Destroy();

        vkDestroySemaphore(_device, resource.imageReadySemaphore, nullptr);
        vkDestroySemaphore(_device, resource.renderCompleteSemaphore, nullptr);
        vkFreeCommandBuffers(_device, _command_pool, 1, &resource.commandBuffer);
        vkDestroyFence(_device, resource.commandFence, nullptr);

        resource = {};
    }

} // namespace Vkxel
