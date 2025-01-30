//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_GUI_H
#define VKXEL_GUI_H

#include <cstdint>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "renderer.h"

namespace Vkxel {
    class Renderer;
    class GUI {
    public:
        void Create(const Renderer& renderer);
        void Render(VkCommandBuffer command_buffer);
        void Destroy();

    private:
        ImGuiContext* _context = nullptr;

    };

} // Vkxel

#endif //VKXEL_GUI_H
