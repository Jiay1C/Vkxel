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
    static constexpr uint32_t StagingBufferSize = 104857600; // 100 MB
    static constexpr uint32_t IndexBufferSize = 104857600; // 100 MB
    static constexpr uint32_t VertexBufferSize = 104857600; // 100 MB
    static constexpr float DefaultMoveSpeed = 1.0f;
    static constexpr float DefaultRotateSpeed = 100.0f;
};

} // Vkxel

#endif //VKXEL_APPLICATION_H
