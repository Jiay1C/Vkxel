//
// Created by jiayi on 2/2/2025.
//

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "buffer.h"
#include "util/check.h"

namespace Vkxel::VkUtil {

    void Buffer::Create() {
        CHECK_RESULT_VK(
                vmaCreateBuffer(allocator, &createInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo));

        if (viewCreateInfo.sType == VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO) {
            viewCreateInfo.buffer = buffer;
            CHECK_RESULT_VK(vkCreateBufferView(device, &viewCreateInfo, nullptr, &bufferView));
        }
    }

    void Buffer::Destroy() {
        if (bufferView) {
            vkDestroyBufferView(device, bufferView, nullptr);
        }
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    std::byte *Buffer::Map() {
        std::byte *memory;
        CHECK_RESULT_VK(vmaMapMemory(allocator, allocation, reinterpret_cast<void **>(&memory)));
        return memory;
    }

    void Buffer::Unmap() { vmaUnmapMemory(allocator, allocation); }

    void Buffer::Flush(const VkDeviceSize offset, const VkDeviceSize size) {
        CHECK_RESULT_VK(vmaFlushAllocation(allocator, allocation, offset, size));
    }

    void Buffer::CmdBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 srcStageMask,
                            VkAccessFlags2 srcAccessMask, VkPipelineStageFlags2 dstStageMask,
                            VkAccessFlags2 dstAccessMask, VkDeviceSize offset, VkDeviceSize size) {
        VkBufferMemoryBarrier2 buffer_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcStageMask = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask = dstStageMask,
                .dstAccessMask = dstAccessMask,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO: Support Queue Family Transfer
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = buffer,
                .offset = offset,
                .size = size};

        VkDependencyInfo dependency_info{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                         .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                                         .bufferMemoryBarrierCount = 1,
                                         .pBufferMemoryBarriers = &buffer_memory_barrier};

        vkCmdPipelineBarrier2(commandBuffer, &dependency_info);
    }


    Buffer BufferBuilder::Build() const {
        Buffer buffer = {.device = _device,
                         .allocator = _allocator,
                         .createInfo = _create_info,
                         .allocationCreateInfo = _allocation_info};

        if (_create_buffer_view) {
            buffer.viewCreateInfo = _view_create_info;
        }

        return buffer;
    }

    BufferBuilder &BufferBuilder::CreateBufferView() {
        _create_buffer_view = true;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetFlags(VkBufferCreateFlags flags) {
        _create_info.flags = flags;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetSize(VkDeviceSize size) {
        _create_info.size = size;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetUsage(VkBufferUsageFlags usage) {
        _create_info.usage = usage;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetSharingMode(VkSharingMode sharingMode) {
        _create_info.sharingMode = sharingMode;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetQueueFamilyIndexCount(uint32_t queueFamilyIndexCount) {
        _create_info.queueFamilyIndexCount = queueFamilyIndexCount;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetPQueueFamilyIndices(const uint32_t *pQueueFamilyIndices) {
        _create_info.pQueueFamilyIndices = pQueueFamilyIndices;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetAllocationFlags(VmaAllocationCreateFlags flags) {
        _allocation_info.flags = flags;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetMemoryUsage(VmaMemoryUsage usage) {
        _allocation_info.usage = usage;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetRequiredFlags(VkMemoryPropertyFlags requiredFlags) {
        _allocation_info.requiredFlags = requiredFlags;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetPreferredFlags(VkMemoryPropertyFlags preferredFlags) {
        _allocation_info.preferredFlags = preferredFlags;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetMemoryTypeBits(uint32_t memoryTypeBits) {
        _allocation_info.memoryTypeBits = memoryTypeBits;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetPool(VmaPool pool) {
        _allocation_info.pool = pool;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetPUserData(void *pUserData) {
        _allocation_info.pUserData = pUserData;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetPriority(float priority) {
        _allocation_info.priority = priority;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetViewFormat(VkFormat format) {
        _view_create_info.format = format;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetViewOffset(VkDeviceSize offset) {
        _view_create_info.offset = offset;
        return *this;
    }

    BufferBuilder &BufferBuilder::SetViewRange(VkDeviceSize range) {
        _view_create_info.range = range;
        return *this;
    }
} // namespace Vkxel::VkUtil
