//
// Created by jiayi on 1/18/2025.
//

#include "vulkan/vulkan.h"

#include "check.h"
#include "input.h"
#include "window.h"

namespace Vkxel {
    uint32_t Window::s_count = 0;

    Window &Window::SetResolution(const uint32_t width, const uint32_t height) {
        _width = width;
        _height = height;
        return *this;
    }
    Window &Window::SetTitle(const std::string_view title) {
        _title = title;
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

        Input::glfwBindWindow(_window);
    }

    void Window::Destroy() {
        glfwDestroyWindow(_window);
        if (--s_count == 0) {
            glfwTerminate();
        }
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

    bool Window::ShouldClose() const { return glfwWindowShouldClose(_window); }

    void Window::RequestClose() const { glfwSetWindowShouldClose(_window, true); }


} // namespace Vkxel
