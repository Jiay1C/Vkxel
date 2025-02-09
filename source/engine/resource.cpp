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

namespace Vkxel {
    ResourceUploader &ResourceUploader::AddObject(const ObjectData &object, ObjectResource &resource) {
        _objects.emplace_back(object, resource);
        return *this;
    }

    void ResourceUploader::UploadObjects() {
        VkDeviceSize total_size = 0;

        for (const auto &object_wrapper: _objects | std::views::keys) {
            const ObjectData &object = object_wrapper;
            total_size += sizeof(IndexType) * object.index.size();
            total_size += sizeof(VertexType) * object.vertex.size();
            total_size += sizeof(ConstantBufferPerObject);
        }

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

            // Upload Index Buffer
            std::ranges::copy(object.index, reinterpret_cast<IndexType *>(host_buffer + host_buffer_offset));
            VkBufferCopy index_copy_region{
                    .srcOffset = host_buffer_offset, .dstOffset = 0, .size = resource.indexBuffer.createInfo.size};
            vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, resource.indexBuffer.buffer, 1, &index_copy_region);
            host_buffer_offset += static_cast<uint32_t>(resource.indexBuffer.createInfo.size);

            // Upload Vertex Buffer
            std::ranges::copy(object.vertex, reinterpret_cast<VertexType *>(host_buffer + host_buffer_offset));
            VkBufferCopy vertex_copy_region{
                    .srcOffset = host_buffer_offset, .dstOffset = 0, .size = resource.vertexBuffer.createInfo.size};
            vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, resource.vertexBuffer.buffer, 1,
                            &vertex_copy_region);
            host_buffer_offset += static_cast<uint32_t>(resource.vertexBuffer.createInfo.size);

            // Upload Constant Buffer
            // Change Matrix To Row Major
            ConstantBufferPerObject constant_buffer_per_object{.transformMatrix =
                                                                       glm::transpose(object.transformMatrix)};
            *reinterpret_cast<ConstantBufferPerObject *>(host_buffer + host_buffer_offset) = constant_buffer_per_object;

            VkBufferCopy constant_copy_region{
                    .srcOffset = host_buffer_offset, .dstOffset = 0, .size = resource.constantBuffer.createInfo.size};
            vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, resource.constantBuffer.buffer, 1,
                            &constant_copy_region);
            host_buffer_offset += static_cast<uint32_t>(resource.constantBuffer.createInfo.size);
        }

        staging_buffer.Flush();

        immediate_command.End();

        staging_buffer.Unmap();
        staging_buffer.Destroy();

        _objects.clear();
    }

    ObjectResource ResourceManager::CreateObjectResource(const ObjectData &object) {

        VkUtil::BufferBuilder bufferBuilder(_device, _allocator);
        bufferBuilder.SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE).SetPQueueFamilyIndices(&_queue_family);

        // Create Index Buffer
        VkUtil::Buffer index_buffer =
                bufferBuilder.SetSize(sizeof(IndexType) * object.index.size())
                        .SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .Build();
        index_buffer.Create();

        // Create Vertex Buffer
        VkUtil::Buffer vertex_buffer =
                bufferBuilder.SetSize(sizeof(VertexType) * object.vertex.size())
                        .SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .Build();
        vertex_buffer.Create();

        // Create Constant Buffer
        // Change Matrix To Row Major
        VkUtil::Buffer constant_buffer =
                bufferBuilder.SetSize(sizeof(ConstantBufferPerObject))
                        .SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .Build();
        constant_buffer.Create();

        // Create DescriptorSet
        VkUtil::DescriptorSet descriptor_set =
                VkUtil::DescriptorSetBuilder(_device, _descriptor_pool, _descriptor_set_layout_object).Build();
        descriptor_set.Create();

        VkDescriptorBufferInfo constant_buffer_per_object_info{
                .buffer = constant_buffer.buffer, .offset = 0, .range = VK_WHOLE_SIZE};
        std::array descriptor_set_write_info = {VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor_set.set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &constant_buffer_per_object_info,
        }};

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptor_set_write_info.size()),
                               descriptor_set_write_info.data(), 0, nullptr);

        return {.indexCount = static_cast<uint32_t>(object.index.size()),
                .firstIndex = 0,
                .indexBuffer = index_buffer,
                .vertexBuffer = vertex_buffer,
                .constantBuffer = constant_buffer,
                .descriptorSet = descriptor_set};
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
                                            .SetFormat(VK_FORMAT_D32_SFLOAT)
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
                        .SetFormat(Application::DefaultFramebufferFormat)
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

        return {.constantBuffer = constant_buffer,
                .colorImage = color_image,
                .depthImage = depth_image,
                .descriptorSet = descriptor_set};
    }

    void ResourceManager::DestroyFrameResource(FrameResource &resource) {
        resource.constantBuffer.Destroy();
        resource.depthImage.Destroy();
        resource.colorImage.Destroy();
        resource.descriptorSet.Destroy();

        resource = {};
    }


} // namespace Vkxel
