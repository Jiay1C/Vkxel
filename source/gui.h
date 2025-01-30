//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_GUI_H
#define VKXEL_GUI_H

#include <cstdint>

#include "vulkan/vulkan.h"
#include "imgui.h"

#include "window.h"

namespace Vkxel {
    struct GUIInitInfo {
        VkInstance                      Instance;
        VkPhysicalDevice                PhysicalDevice;
        VkDevice                        Device;
        uint32_t                        QueueFamily;
        VkQueue                         Queue;
        VkDescriptorPool                DescriptorPool;
        uint32_t                        MinImageCount;
        uint32_t                        ImageCount;
        VkFormat                        ColorAttachmentFormat;
    };

    class GUI {
    public:
        explicit GUI(Window& window) : _window(window) {}

        void InitVK(const GUIInitInfo* pInfo);
        void Render(VkCommandBuffer commandBuffer);
        void Update();
        void DestroyVK();

    private:
        void OnGUI();

        Window& _window;
        ImGuiContext* _context = nullptr;
    };

} // Vkxel

#endif //VKXEL_GUI_H
