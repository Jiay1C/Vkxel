//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_GUI_H
#define VKXEL_GUI_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "imgui.h"
#include "vulkan/vulkan.h"

#include "window.h"

namespace Vkxel {

    struct GuiInitInfo {
        VkInstance Instance;
        VkPhysicalDevice PhysicalDevice;
        VkDevice Device;
        uint32_t QueueFamily;
        VkQueue Queue;
        VkDescriptorPool DescriptorPool;
        uint32_t MinImageCount;
        uint32_t ImageCount;
        VkFormat ColorAttachmentFormat;
    };

    class GUI {
    public:
        using GuiDelegate = Delegate<>;

        explicit GUI(Window &window) : _window(window) {}

        void InitVK(const GuiInitInfo *pInfo);
        void Render(VkCommandBuffer commandBuffer);
        void Update();
        void DestroyVK();

        void AddItem(const std::string &guiWindow, const GuiDelegate::Callback &item);
        void RemoveWindow(const std::string &guiWindow);

    private:
        void OnGUI();
        void ApplyContext();
        void RestoreContext();

        std::unordered_map<std::string, GuiDelegate> _gui_window;

        Window &_window;

        ImGuiContext *_context = nullptr;
        ImGuiContext *_last_context = nullptr;
    };

} // namespace Vkxel

#endif // VKXEL_GUI_H
