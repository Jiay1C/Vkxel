//
// Created by jiayi on 2/2/2025.
//

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "image.h"
#include "util/check.h"

namespace Vkxel::VkUtil {

    void Image::Create() {
        CHECK_RESULT_VK(
                vmaCreateImage(allocator, &createInfo, &allocationCreateInfo, &image, &allocation, &allocationInfo));

        if (viewCreateInfo.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO) {
            viewCreateInfo.image = image;
            CHECK_RESULT_VK(vkCreateImageView(device, &viewCreateInfo, nullptr, &imageView));
        }
    }


    void Image::Destroy() {
        if (imageView) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        vmaDestroyImage(allocator, image, allocation);
    }

    std::byte *Image::Map() {
        std::byte *memory;
        CHECK_RESULT_VK(vmaMapMemory(allocator, allocation, reinterpret_cast<void **>(&memory)));
        return memory;
    }

    void Image::Unmap() { vmaUnmapMemory(allocator, allocation); }

    void Image::Flush(const VkDeviceSize offset, const VkDeviceSize size) {
        CHECK_RESULT_VK(vmaFlushAllocation(allocator, allocation, offset, size));
    }

    void Image::CmdBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 srcStageMask,
                           VkAccessFlags2 srcAccessMask, VkPipelineStageFlags2 dstStageMask,
                           VkAccessFlags2 dstAccessMask, VkImageLayout newLayout) {
        CHECK(imageView, "Currently CmdBarrier Only Support Image With ImageView");

        VkImageMemoryBarrier2 image_memory_barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask = dstStageMask,
                .dstAccessMask = dstAccessMask,
                .oldLayout = createInfo.initialLayout,
                .newLayout = newLayout == VK_IMAGE_LAYOUT_UNDEFINED ? createInfo.initialLayout : newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // TODO: Support Queue Family Transfer
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image,
                .subresourceRange = viewCreateInfo.subresourceRange};

        VkDependencyInfo dependency_info{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                         .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                                         .imageMemoryBarrierCount = 1,
                                         .pImageMemoryBarriers = &image_memory_barrier};

        vkCmdPipelineBarrier2(commandBuffer, &dependency_info);

        createInfo.initialLayout = image_memory_barrier.newLayout; // TODO: More Precise Layout Update
    }


    Image ImageBuilder::Build() const {
        Image image = {.device = _device,
                       .allocator = _allocator,
                       .createInfo = _create_info,
                       .allocationCreateInfo = _allocation_info};

        if (_create_image_view) {
            image.viewCreateInfo = _view_create_info;
        }

        return image;
    }

    ImageBuilder &ImageBuilder::CreateImageView() {
        _create_image_view = true;
        return *this;
    }


    ImageBuilder &ImageBuilder::SetFlags(VkImageCreateFlags flags) {
        _create_info.flags = flags;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetPNext(const void *pNext) {
        _create_info.pNext = pNext;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetImageType(VkImageType imageType) {
        _create_info.imageType = imageType;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetFormat(VkFormat format) {
        _create_info.format = format;
        _view_create_info.format = format;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetExtent(VkExtent3D extent) {
        _create_info.extent = extent;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetMipLevels(uint32_t mipLevels) {
        _create_info.mipLevels = mipLevels;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetArrayLayers(uint32_t arrayLayers) {
        _create_info.arrayLayers = arrayLayers;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetSamples(VkSampleCountFlagBits samples) {
        _create_info.samples = samples;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetTiling(VkImageTiling tiling) {
        _create_info.tiling = tiling;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetUsage(VkImageUsageFlags usage) {
        _create_info.usage = usage;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetSharingMode(VkSharingMode sharingMode) {
        _create_info.sharingMode = sharingMode;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetQueueFamilyIndexCount(uint32_t queueFamilyIndexCount) {
        _create_info.queueFamilyIndexCount = queueFamilyIndexCount;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetPQueueFamilyIndices(const uint32_t *pQueueFamilyIndices) {
        _create_info.pQueueFamilyIndices = pQueueFamilyIndices;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetLayout(VkImageLayout layout) {
        _create_info.initialLayout = layout;
        return *this;
    }

    // VmaAllocationCreateInfo Setters
    ImageBuilder &ImageBuilder::SetAllocationFlags(VmaAllocationCreateFlags flags) {
        _allocation_info.flags = flags;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetMemoryUsage(VmaMemoryUsage usage) {
        _allocation_info.usage = usage;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetRequiredFlags(VkMemoryPropertyFlags requiredFlags) {
        _allocation_info.requiredFlags = requiredFlags;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetPreferredFlags(VkMemoryPropertyFlags preferredFlags) {
        _allocation_info.preferredFlags = preferredFlags;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetMemoryTypeBits(uint32_t memoryTypeBits) {
        _allocation_info.memoryTypeBits = memoryTypeBits;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetPool(VmaPool pool) {
        _allocation_info.pool = pool;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetPUserData(void *pUserData) {
        _allocation_info.pUserData = pUserData;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetPriority(float priority) {
        _allocation_info.priority = priority;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetViewFlags(VkImageViewCreateFlags flags) {
        _view_create_info.flags = flags;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetViewPNext(const void *pNext) {
        _view_create_info.pNext = pNext;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetViewType(VkImageViewType viewType) {
        _view_create_info.viewType = viewType;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetViewFormat(VkFormat format) {
        _view_create_info.format = format;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetViewComponents(VkComponentMapping components) {
        _view_create_info.components = components;
        return *this;
    }

    ImageBuilder &ImageBuilder::SetViewSubresourceRange(VkImageSubresourceRange subresourceRange) {
        _view_create_info.subresourceRange = subresourceRange;
        return *this;
    }
} // namespace Vkxel::VkUtil
