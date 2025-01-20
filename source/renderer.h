//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_RENDERER_H
#define VKXEL_RENDERER_H

#include "vulkan/vulkan.h"
#include "VkBootstrap.h"

#include "window.h"

namespace Vkxel {

class Renderer {
public:
    void Init();
    void Destroy();

    void Allocate();
    void Release();

    bool Render();

private:
    Window _window;

    // Device Related Handle

    vkb::Instance _instance;
    vkb::PhysicalDevice _physical_device;
    vkb::Device _device;
    vkb::Swapchain _swapchain;

    VkSurfaceKHR _surface = nullptr;
    VkQueue _queue = nullptr;
    VkCommandPool _command_pool = nullptr;


    // Resource Related Handle

    VkPipeline _pipeline = nullptr;


};

} // Vkxel

#endif //VKXEL_RENDERER_H
