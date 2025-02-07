//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_WINDOW_H
#define VKXEL_WINDOW_H

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "util/delegate.hpp"

namespace Vkxel {

    enum class WindowEvent {
        Create,
        Minimize,
        Restore,
        Resize,
        Destroy,
    };


    class Window {
    public:
        using WindowEventDelegate = Delegate<>;

        Window &SetSize(uint32_t width, uint32_t height);
        Window &SetTitle(std::string_view title);
        Window &AddCallback(WindowEvent event, const WindowEventDelegate::Callback &callback);

        void Create();
        void Destroy();

        VkSurfaceKHR CreateSurface(VkInstance instance);
        void DestroySurface();

        GLFWwindow *GetWindow() const;
        VkSurfaceKHR GetSurface() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        float GetAspect() const;

        uint32_t GetFrameBufferWidth() const;
        uint32_t GetFrameBufferHeight() const;

        bool ShouldClose() const;
        void RequestClose() const;

        void Update();

    private:
        void TriggerCallback(WindowEvent event);

        static uint32_t s_count;

        uint32_t _width = 0;
        uint32_t _height = 0;

        uint32_t _framebuffer_width = 0;
        uint32_t _framebuffer_height = 0;

        std::string _title;

        VkInstance _instance = nullptr;
        VkSurfaceKHR _surface = nullptr;
        GLFWwindow *_window = nullptr;

        std::unordered_map<WindowEvent, WindowEventDelegate> _callbacks;
    };

} // namespace Vkxel

#endif // VKXEL_WINDOW_H
