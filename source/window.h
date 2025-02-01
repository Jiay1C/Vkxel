//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_WINDOW_H
#define VKXEL_WINDOW_H

#include <cstdint>
#include <string>
#include <string_view>

#include "GLFW/glfw3.h"
#include "Vulkan/vulkan.h"

namespace Vkxel {

    class Window {
    public:
        Window &SetResolution(uint32_t width, uint32_t height);
        Window &SetTitle(std::string_view title);

        void Create();
        void Destroy();

        VkSurfaceKHR CreateSurface(VkInstance instance);
        void DestroySurface();

        GLFWwindow *GetWindow() const;
        VkSurfaceKHR GetSurface() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        bool ShouldClose() const;
        void RequestClose() const;

    private:
        static uint32_t s_count;

        uint32_t _width = 0;
        uint32_t _height = 0;
        std::string _title;

        VkInstance _instance = nullptr;
        VkSurfaceKHR _surface = nullptr;
        GLFWwindow *_window = nullptr;
    };

} // namespace Vkxel

#endif // VKXEL_WINDOW_H
