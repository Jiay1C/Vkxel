//
// Created by jiayi on 1/18/2025.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <cstdint>
#include <string>
#include <string_view>

#include "Vulkan/vulkan.h"
#include "GLFW/glfw3.h"

namespace Vkxel {

class Window {
public:
    void SetResolution(uint32_t width, uint32_t height);
    void SetTitle(std::string_view title);
    void SetInstance(VkInstance instance);

    void Create();
    void Destroy();

    GLFWwindow* GetWindow() const;
    VkSurfaceKHR GetSurface() const;

    bool Update() const;

private:
    uint32_t _width = 0;
    uint32_t _height = 0;
    std::string _title;

    VkInstance _instance = nullptr;
    VkSurfaceKHR _surface = nullptr;
    GLFWwindow* _window = nullptr;
};

} // Vkxel

#endif //WINDOW_H
