//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_APPLICATION_H
#define VKXEL_APPLICATION_H

#include <cstdint>
#include <string_view>
#include <utility>

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

namespace Vkxel {

    class Application {
    public:
        Application() = delete;
        ~Application() = delete;

        static constexpr std::string_view Name = "Vkxel";
        static constexpr std::uint32_t Version = 0;
        static constexpr uint32_t VulkanVersion = VK_API_VERSION_1_3;

        static constexpr uint32_t BackgroundModeMaxFps = 10;

        static constexpr uint32_t DefaultWindowWidth = 1280;
        static constexpr uint32_t DefaultWindowHeight = 720;

        static constexpr float DefaultFov = glm::radians(75.0f);
        static constexpr float DefaultNearClipPlane = 0.01f;
        static constexpr float DefaultFarClipPlane = 1000.0f;

        static constexpr VkPresentModeKHR DefaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        static constexpr VkFormat DefaultFramebufferFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

        static constexpr uint32_t DefaultDescriptorCount = 1000;
        static constexpr uint32_t DefaultDescriptorSetCount = 1000;

        static constexpr float DefaultMoveSpeed = 1.0f;
        static constexpr float DefaultRotateSpeed = 1.0f;
        static constexpr float DefaultAccelerateRatio = 5.0f;

        static constexpr std::string_view DefaultCanvasPanelName = "Canvas";
    };

} // namespace Vkxel

#endif // VKXEL_APPLICATION_H
