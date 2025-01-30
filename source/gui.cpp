//
// Created by jiayi on 1/26/2025.
//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "gui.h"
#include "window.h"
#include "input.h"

namespace Vkxel {

    void GUI::OnGUI() {
        ImGui::ShowDemoWindow();
    }

    void GUI::InitVK(const GUIInitInfo* pInfo) {
        IMGUI_CHECKVERSION();
        _context = ImGui::CreateContext();
        ImGui::SetCurrentContext(_context);
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(_window.GetWindow(), true);

        VkPipelineRenderingCreateInfo ui_rendering_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &(pInfo->ColorAttachmentFormat),
        };

        ImGui_ImplVulkan_InitInfo imgui_init_info = {
            .Instance = pInfo->Instance,
            .PhysicalDevice = pInfo->PhysicalDevice,
            .Device = pInfo->Device,
            .QueueFamily = pInfo->QueueFamily,
            .Queue = pInfo->Queue,
            .DescriptorPool = pInfo->DescriptorPool,
            .MinImageCount = pInfo->MinImageCount,
            .ImageCount = pInfo->ImageCount,
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = ui_rendering_create_info
        };

        ImGui_ImplVulkan_Init(&imgui_init_info);
    }


    void GUI::Render(VkCommandBuffer commandBuffer) {
        ImGui::SetCurrentContext(_context);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        OnGUI();

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    }

    void GUI::Update() {
        if (Input::GetLastInputWindow() == _window.GetWindow()) {
            const ImGuiIO & io = ImGui::GetIO();
            Input::EnableKeyboardInput(!io.WantCaptureKeyboard);
            Input::EnableMouseInput(!io.WantCaptureMouse);
        }
    }

    void GUI::DestroyVK() {
        ImGui::SetCurrentContext(_context);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(_context);
    }

} // Vkxel