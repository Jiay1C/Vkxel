//
// Created by jiayi on 2/6/2025.
//

#ifndef VKXEL_RESOURCE_H
#define VKXEL_RESOURCE_H

#include <cstdint>

#include "vkutil/buffer.h"

namespace Vkxel {
    struct MaterialResource {};

    struct ObjectResource {
        uint32_t indexCount = 0;
        uint32_t firstIndex = 0;
        VkUtil::Buffer indexBuffer = {};
        VkUtil::Buffer vertexBuffer = {};
        VkUtil::Buffer constantBuffer = {};
        VkDescriptorSet descriptorSet = nullptr;
    };

    struct FrameResource {
        VkUtil::Buffer constantBuffer = {};
    };

} // namespace Vkxel

#endif // VKXEL_RESOURCE_H
