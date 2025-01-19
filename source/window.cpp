//
// Created by jiayi on 1/18/2025.
//

#include "vulkan/vulkan.h"

#include "window.h"
#include "check.h"

namespace Vkxel {
    Window& Window::SetResolution(const uint32_t width, const uint32_t height) {
        _width = width;
        _height = height;
        return *this;
    }
    Window& Window::SetTitle(const std::string_view title) {
        _title = title;
        return *this;
    }
    Window& Window::SetInstance(const VkInstance instance) {
        _instance = instance;
        return *this;
    }

    void Window::Create() {
        CHECK_RESULT(GLFW_TRUE, glfwInit());

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(_width, _height, _title.data(), nullptr, nullptr);
        CHECK_NOTNULL(_window);

        CHECK_RESULT_VK(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface));
    }

    void Window::Destroy() {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    GLFWwindow *Window::GetWindow() const {
        return _window;
    }


    VkSurfaceKHR Window::GetSurface() const {
        return _surface;
    }

    bool Window::Update() const {
        glfwPollEvents();
        return glfwWindowShouldClose(_window);
    }


} // Vkxel