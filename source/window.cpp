//
// Created by jiayi on 1/18/2025.
//

#include "vulkan/vulkan.h"

#include "check.h"
#include "input.h"
#include "window.h"

namespace Vkxel {
    uint32_t Window::s_count = 0;

    Window &Window::SetSize(const uint32_t width, const uint32_t height) {
        _width = width;
        _height = height;
        return *this;
    }

    Window &Window::SetTitle(const std::string_view title) {
        _title = title;
        return *this;
    }

    Window &Window::AddCallback(const WindowEvent event, const WindowEventDelegate::Callback &callback) {
        _callbacks[event] += callback;
        return *this;
    }

    void Window::Create() {
        if (s_count++ == 0) {
            CHECK_RESULT(GLFW_TRUE, glfwInit());
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
        CHECK_NOTNULL(_window);

        int framebuffer_width, framebuffer_height;
        glfwGetFramebufferSize(_window, &framebuffer_width, &framebuffer_height);
        _framebuffer_width = framebuffer_width;
        _framebuffer_height = framebuffer_height;

        Input::glfwBindWindow(_window);

        TriggerCallback(WindowEvent::Create);
    }

    void Window::Destroy() {
        glfwDestroyWindow(_window);
        if (--s_count == 0) {
            glfwTerminate();
        }

        TriggerCallback(WindowEvent::Destroy);
    }

    VkSurfaceKHR Window::CreateSurface(const VkInstance instance) {
        _instance = instance;
        CHECK_RESULT_VK(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface));
        return _surface;
    }

    void Window::DestroySurface() {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _surface = nullptr;
    }


    GLFWwindow *Window::GetWindow() const { return _window; }


    VkSurfaceKHR Window::GetSurface() const { return _surface; }

    uint32_t Window::GetWidth() const { return _width; }

    uint32_t Window::GetHeight() const { return _height; }

    uint32_t Window::GetFrameBufferWidth() const { return _framebuffer_width; }

    uint32_t Window::GetFrameBufferHeight() const { return _framebuffer_height; }


    bool Window::ShouldClose() const { return glfwWindowShouldClose(_window); }

    void Window::RequestClose() const { glfwSetWindowShouldClose(_window, true); }

    void Window::Update() {
        int new_framebuffer_width, new_framebuffer_height, new_width, new_height;
        glfwGetFramebufferSize(_window, &new_framebuffer_width, &new_framebuffer_height);
        glfwGetWindowSize(_window, &new_width, &new_height);

        bool minimized = (_framebuffer_width == 0 || _framebuffer_height == 0);
        bool new_minimized = (new_framebuffer_width == 0 || new_framebuffer_height == 0);

        if (minimized && !new_minimized) {
            TriggerCallback(WindowEvent::Restore);
        } else if (!minimized && new_minimized) {
            TriggerCallback(WindowEvent::Minimize);
        } else if (_width != new_width || _height != new_height || _framebuffer_width != new_framebuffer_width ||
                   _framebuffer_height != new_framebuffer_height) {
            TriggerCallback(WindowEvent::Resize);
        }

        _framebuffer_width = new_framebuffer_width;
        _framebuffer_height = new_framebuffer_height;
        _width = new_width;
        _height = new_height;
    }

    void Window::TriggerCallback(const WindowEvent event) {
        if (_callbacks.contains(event)) {
            _callbacks.at(event)();
        }
    }

} // namespace Vkxel
