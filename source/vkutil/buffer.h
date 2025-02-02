//
// Created by jiayi on 2/2/2025.
//

#ifndef VKXEL_BUFFER_H
#define VKXEL_BUFFER_H

#include <utility>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace Vkxel::VkUtil {
    struct Buffer {
        VkDevice device = nullptr;
        VmaAllocator allocator = nullptr;
        VkBuffer buffer = nullptr;
        VkBufferCreateInfo bufferCreateInfo = {};
        VmaAllocation allocation = nullptr;
        VmaAllocationCreateInfo allocationCreateInfo = {};
        VmaAllocationInfo allocationInfo = {};
        VkBufferView bufferView = nullptr;
        VkBufferViewCreateInfo bufferViewCreateInfo = {};

        void Create();
        void Destroy();

        std::byte *Map();
        void Unmap();
        void Flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

        void CmdBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
                        VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkDeviceSize offset = 0,
                        VkDeviceSize size = VK_WHOLE_SIZE);
    };

    class BufferBuilder {
    public:
        explicit BufferBuilder(const VkDevice device, const VmaAllocator allocator) :
            _device(device), _allocator(allocator) {}

        explicit BufferBuilder(const Buffer &oldBuffer) :
            _device(oldBuffer.device), _allocator(oldBuffer.allocator), _create_info(oldBuffer.bufferCreateInfo),
            _view_create_info(oldBuffer.bufferViewCreateInfo), _allocation_info(oldBuffer.allocationCreateInfo),
            _create_buffer_view(oldBuffer.bufferView != nullptr) {}

        Buffer Build() const;

        BufferBuilder &CreateBufferView();

        BufferBuilder &SetFlags(VkBufferCreateFlags flags);
        BufferBuilder &SetSize(VkDeviceSize size);
        BufferBuilder &SetUsage(VkBufferUsageFlags usage);
        BufferBuilder &SetSharingMode(VkSharingMode sharingMode);
        BufferBuilder &SetQueueFamilyIndexCount(uint32_t queueFamilyIndexCount);
        BufferBuilder &SetPQueueFamilyIndices(const uint32_t *pQueueFamilyIndices);

        BufferBuilder &SetAllocationFlags(VmaAllocationCreateFlags flags);
        BufferBuilder &SetMemoryUsage(VmaMemoryUsage usage);
        BufferBuilder &SetRequiredFlags(VkMemoryPropertyFlags requiredFlags);
        BufferBuilder &SetPreferredFlags(VkMemoryPropertyFlags preferredFlags);
        BufferBuilder &SetMemoryTypeBits(uint32_t memoryTypeBits);
        BufferBuilder &SetPool(VmaPool pool);
        BufferBuilder &SetPUserData(void *pUserData);
        BufferBuilder &SetPriority(float priority);

        BufferBuilder &SetViewFormat(VkFormat format);
        BufferBuilder &SetViewOffset(VkDeviceSize offset);
        BufferBuilder &SetViewRange(VkDeviceSize range);

    private:
        VkDevice _device = nullptr;
        VmaAllocator _allocator = nullptr;
        bool _create_buffer_view = false;

        VkBufferCreateInfo _create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                        .pNext = nullptr,
                                        .flags = 0,
                                        .size = 0,
                                        .usage = 0,
                                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                        .queueFamilyIndexCount = 1,
                                        .pQueueFamilyIndices = nullptr};

        VmaAllocationCreateInfo _allocation_info{
                .flags = 0,
                .usage = VMA_MEMORY_USAGE_AUTO,
                .requiredFlags = 0,
                .preferredFlags = 0,
                .memoryTypeBits = 0,
                .pool = nullptr,
                .pUserData = nullptr,
                .priority = 0,
        };

        VkBufferViewCreateInfo _view_create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                                                 .pNext = nullptr,
                                                 .flags = 0,
                                                 .buffer = nullptr,
                                                 .format = VK_FORMAT_UNDEFINED,
                                                 .offset = 0,
                                                 .range = VK_WHOLE_SIZE};
    };
} // namespace Vkxel::VkUtil

#endif // VKXEL_BUFFER_H
