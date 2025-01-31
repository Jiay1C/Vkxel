//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_GUI_H
#define VKXEL_GUI_H

#include <cstdint>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <string>

#include "vulkan/vulkan.h"
#include "imgui.h"

#include "window.h"

namespace Vkxel {

    using GuiItem = std::function<void(void)>;

    struct GuiInitInfo {
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

        void InitVK(const GuiInitInfo* pInfo);
        void Render(VkCommandBuffer commandBuffer);
        void Update();
        void DestroyVK();

        void AddStaticItem(const std::string& guiWindow, const GuiItem& item);
        void AddDynamicItem(const std::string& guiWindow, const GuiItem& item);

    private:
        void OnGUI();
        void ApplyContext();
        void RestoreContext();

        struct GuiWindow {
            std::vector<GuiItem> StaticItem;
            std::queue<GuiItem> DynamicItem;
        };

        std::unordered_map<std::string, GuiWindow> _gui_window;

        Window& _window;

        ImGuiContext* _context = nullptr;
        ImGuiContext* _last_context = nullptr;
    };

} // Vkxel

#endif //VKXEL_GUI_H
