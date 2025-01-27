//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_APPLICATION_H
#define VKXEL_APPLICATION_H

#include <cstdint>
#include <string_view>
#include <utility>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

namespace Vkxel {

class Application {
public:
    Application()=delete;
    ~Application()=delete;

    static constexpr std::string_view Name = "Vkxel";
    static constexpr std::uint32_t Version = 0;
    static constexpr uint32_t VulkanVersion = VK_API_VERSION_1_3;
    static constexpr std::pair<uint32_t, uint32_t> DefaultResolution = {1280, 720};
    static constexpr float DefaultFov = glm::radians(60.0f);
    static constexpr std::pair<float, float> DefaultClipPlane = {0.01f, 1000.0f};
    static constexpr VkPresentModeKHR DefaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    static constexpr uint32_t DefaultStagingBufferSize = 104857600; // 100 MB
    static constexpr uint32_t DefaultIndexBufferSize = 104857600; // 100 MB
    static constexpr uint32_t DefaultVertexBufferSize = 104857600; // 100 MB
    static constexpr uint32_t DefaultDescriptorCount = 1000;
    static constexpr uint32_t DefaultDescriptorSetCount = 1000;
    static constexpr float DefaultMoveSpeed = 1.0f;
    static constexpr float DefaultRotateSpeed = 1.0f;
};

} // Vkxel

#endif //VKXEL_APPLICATION_H
