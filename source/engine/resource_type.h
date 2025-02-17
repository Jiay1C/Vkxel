//
// Created by jiayi on 2/6/2025.
//

#ifndef VKXEL_RESOURCE_TYPE_H
#define VKXEL_RESOURCE_TYPE_H

#include <cstdint>

#include "vkutil/buffer.h"
#include "vkutil/descriptor.h"
#include "vkutil/image.h"

namespace Vkxel {
    struct MaterialResource {};

    struct ObjectResource {
        bool isActive = false;
        uint32_t indexCount = 0;
        uint32_t firstIndex = 0;
        VkUtil::Buffer indexBuffer = {};
        VkUtil::Buffer vertexBuffer = {};
        VkUtil::Buffer constantBuffer = {};
        VkUtil::DescriptorSet descriptorSet = {};
    };

    struct FrameResource {
        VkUtil::Buffer constantBuffer = {};

        VkUtil::Image colorImage = {};
        VkUtil::Image depthImage = {};

        VkUtil::DescriptorSet descriptorSet = {};
    };

} // namespace Vkxel

#endif // VKXEL_RESOURCE_TYPE_H
