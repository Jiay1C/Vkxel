//
// Created by jiayi on 1/26/2025.
//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "gui.h"
#include "input.h"
#include "window.h"

namespace Vkxel {

    void GUI::OnGUI() {
        ImGui::ShowDemoWindow();

        for (auto &[gui_window, gui_item]: _gui_window) {
            ImGui::Begin(gui_window.data());

            // Static Items
            for (auto &item: gui_item.StaticItem) {
                item();
            }

            // Dynamic Items
            while (!gui_item.DynamicItem.empty()) {
                gui_item.DynamicItem.front()();
                gui_item.DynamicItem.pop();
            }

            ImGui::End();
        }
    }

    void GUI::InitVK(const GuiInitInfo *pInfo) {
        IMGUI_CHECKVERSION();

        _context = ImGui::CreateContext();
        ApplyContext();

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(_window.GetWindow(), true);

        ImGui_ImplVulkan_InitInfo imgui_init_info = {.Instance = pInfo->Instance,
                                                     .PhysicalDevice = pInfo->PhysicalDevice,
                                                     .Device = pInfo->Device,
                                                     .QueueFamily = pInfo->QueueFamily,
                                                     .Queue = pInfo->Queue,
                                                     .DescriptorPool = pInfo->DescriptorPool,
                                                     .MinImageCount = pInfo->MinImageCount,
                                                     .ImageCount = pInfo->ImageCount,
                                                     .UseDynamicRendering = true,
                                                     .PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfo{
                                                             .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                                             .colorAttachmentCount = 1,
                                                             .pColorAttachmentFormats = &(pInfo->ColorAttachmentFormat),
                                                     }};

        ImGui_ImplVulkan_Init(&imgui_init_info);

        RestoreContext();
    }


    void GUI::Render(VkCommandBuffer commandBuffer) {
        ApplyContext();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        OnGUI();

        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

        RestoreContext();
    }

    void GUI::Update() {
        if (Input::GetLastInputWindow() == _window.GetWindow()) {
            ImGui::SetCurrentContext(_context);
            const ImGuiIO &io = ImGui::GetIO();
            Input::EnableKeyboardInput(!io.WantCaptureKeyboard);
            Input::EnableMouseInput(!io.WantCaptureMouse);
        }
    }

    void GUI::DestroyVK() {
        ApplyContext();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(_context);
        RestoreContext();
    }

    void GUI::AddStaticItem(const std::string &guiWindow, const GuiItem &item) {
        _gui_window[guiWindow].StaticItem.push_back(item);
    }

    void GUI::AddDynamicItem(const std::string &guiWindow, const GuiItem &item) {
        _gui_window[guiWindow].DynamicItem.push(item);
    }

    void GUI::ApplyContext() {
        _last_context = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(_context);
    }

    void GUI::RestoreContext() { ImGui::SetCurrentContext(_last_context); }


} // namespace Vkxel
