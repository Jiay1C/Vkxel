//
// Created by jiayi on 1/26/2025.
//

#include "renderer.h"
#include "window.h"
#include "gui.h"

namespace Vkxel {
    void GUI::Create(const Renderer& renderer) {
        IMGUI_CHECKVERSION();
        _context = ImGui::CreateContext();
        ImGui::SetCurrentContext(_context);
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(renderer.GetWindow().GetWindow(), true);

        VkPipelineRenderingCreateInfo ui_rendering_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &(renderer._swapchain.image_format),
        };

        ImGui_ImplVulkan_InitInfo imgui_init_info = {
            .Instance = renderer._instance,
            .PhysicalDevice = renderer._physical_device,
            .Device = renderer._device,
            .QueueFamily = renderer._device.get_queue_index(vkb::QueueType::graphics).value(),
            .Queue = renderer._queue,
            .DescriptorPool = renderer._descriptor_pool,
            .MinImageCount = renderer._swapchain.requested_min_image_count,
            .ImageCount = renderer._swapchain.image_count,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .DescriptorPoolSize = 0,
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = ui_rendering_create_info
        };

        ImGui_ImplVulkan_Init(&imgui_init_info);
    }


    void GUI::Render(VkCommandBuffer command_buffer) {
        ImGui::SetCurrentContext(_context);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
    }



    void GUI::Destroy() {
        ImGui::SetCurrentContext(_context);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(_context);
    }



} // Vkxel