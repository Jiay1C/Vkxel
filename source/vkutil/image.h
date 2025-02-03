//
// Created by jiayi on 2/2/2025.
//

#ifndef VKXEL_IMAGE_H
#define VKXEL_IMAGE_H

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace Vkxel::VkUtil {
    struct Image {
        VkDevice device = nullptr;
        VmaAllocator allocator = nullptr;
        VkImage image = nullptr;
        VkImageCreateInfo imageCreateInfo = {};
        VmaAllocation allocation = nullptr;
        VmaAllocationCreateInfo allocationCreateInfo = {};
        VmaAllocationInfo allocationInfo = {};
        VkImageView imageView = nullptr;
        VkImageViewCreateInfo imageViewCreateInfo = {};

        void Create();
        void Destroy();

        std::byte *Map();
        void Unmap();
        void Flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

        void CmdBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
                        VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
                        VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED);
    };

    class ImageBuilder {
    public:
        explicit ImageBuilder(const VkDevice device, const VmaAllocator allocator) :
            _device(device), _allocator(allocator) {}

        explicit ImageBuilder(const Image &oldImage) :
            _device(oldImage.device), _allocator(oldImage.allocator), _create_info(oldImage.imageCreateInfo),
            _view_create_info(oldImage.imageViewCreateInfo), _allocation_info(oldImage.allocationCreateInfo),
            _create_image_view(oldImage.imageView != nullptr) {}

        Image Build() const;

        ImageBuilder &CreateImageView();

        ImageBuilder &SetFlags(VkImageCreateFlags flags);
        ImageBuilder &SetPNext(const void *pNext);
        ImageBuilder &SetImageType(VkImageType imageType);
        ImageBuilder &SetFormat(VkFormat format);
        ImageBuilder &SetExtent(VkExtent3D extent);
        ImageBuilder &SetMipLevels(uint32_t mipLevels);
        ImageBuilder &SetArrayLayers(uint32_t arrayLayers);
        ImageBuilder &SetSamples(VkSampleCountFlagBits samples);
        ImageBuilder &SetTiling(VkImageTiling tiling);
        ImageBuilder &SetUsage(VkImageUsageFlags usage);
        ImageBuilder &SetSharingMode(VkSharingMode sharingMode);
        ImageBuilder &SetQueueFamilyIndexCount(uint32_t queueFamilyIndexCount);
        ImageBuilder &SetPQueueFamilyIndices(const uint32_t *pQueueFamilyIndices);
        ImageBuilder &SetLayout(VkImageLayout layout);

        ImageBuilder &SetAllocationFlags(VmaAllocationCreateFlags flags);
        ImageBuilder &SetMemoryUsage(VmaMemoryUsage usage);
        ImageBuilder &SetRequiredFlags(VkMemoryPropertyFlags requiredFlags);
        ImageBuilder &SetPreferredFlags(VkMemoryPropertyFlags preferredFlags);
        ImageBuilder &SetMemoryTypeBits(uint32_t memoryTypeBits);
        ImageBuilder &SetPool(VmaPool pool);
        ImageBuilder &SetPUserData(void *pUserData);
        ImageBuilder &SetPriority(float priority);

        ImageBuilder &SetViewFlags(VkImageViewCreateFlags flags);
        ImageBuilder &SetViewPNext(const void *pNext);
        ImageBuilder &SetViewType(VkImageViewType viewType);
        ImageBuilder &SetViewFormat(VkFormat format);
        ImageBuilder &SetViewComponents(VkComponentMapping components);
        ImageBuilder &SetViewSubresourceRange(VkImageSubresourceRange subresourceRange);

    private:
        VkDevice _device = nullptr;
        VmaAllocator _allocator = nullptr;

        bool _create_image_view = false;

        VkImageCreateInfo _create_info{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                       .pNext = nullptr,
                                       .flags = 0,
                                       .imageType = VK_IMAGE_TYPE_2D,
                                       .format = VK_FORMAT_UNDEFINED,
                                       .extent = VkExtent3D{.width = 1, .height = 1, .depth = 1},
                                       .mipLevels = 1,
                                       .arrayLayers = 1,
                                       .samples = VK_SAMPLE_COUNT_1_BIT,
                                       .tiling = VK_IMAGE_TILING_OPTIMAL,
                                       .usage = 0,
                                       .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                       .queueFamilyIndexCount = 1,
                                       .pQueueFamilyIndices = nullptr,
                                       .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

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

        VkImageViewCreateInfo _view_create_info{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                                .pNext = nullptr,
                                                .flags = 0,
                                                .image = nullptr,
                                                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                .format = VK_FORMAT_UNDEFINED,
                                                .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                               .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                               .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                               .a = VK_COMPONENT_SWIZZLE_IDENTITY},
                                                .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_NONE,
                                                                     .baseMipLevel = 0,
                                                                     .levelCount = 1,
                                                                     .baseArrayLayer = 0,
                                                                     .layerCount = 1}};
    };
} // namespace Vkxel::VkUtil


#endif // VKXEL_IMAGE_H
