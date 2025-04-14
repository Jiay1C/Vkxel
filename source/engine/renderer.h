//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_RENDERER_H
#define VKXEL_RENDERER_H

#include <array>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "compute.h"
#include "gui.h"
#include "resource.h"
#include "resource_type.h"
#include "vkutil/pipeline.h"
#include "window.h"
#include "world/scene.h"

namespace Vkxel {

    class Renderer {
    public:
        friend class GUI;

        explicit Renderer(Window &window, GUI &gui);

        void Init();
        void Destroy();

        void LoadScene(const Scene &scene);
        void UnloadScene();

        void Render();

        void Resize();

        void WaitIdle() const;

        ComputeJob CreateComputeJob();

        Window &GetWindow() const;
        GUI &GetGUI() const;

    private:
        Window &_window;
        GUI &_gui;

        bool _init = false;
        bool _pause = false;

        RenderContext _context = {};

        // Device Related Handle

        vkb::Instance _instance;
        vkb::PhysicalDevice _physical_device;
        vkb::Device _device;

        vkb::Swapchain _swapchain;
        std::vector<VkImage> _swapchain_image;

        VmaAllocator _allocator = nullptr;

        VkSurfaceKHR _surface = nullptr;
        VkQueue _queue = nullptr;
        VkCommandPool _command_pool = nullptr;
        VkDescriptorPool _descriptor_pool = nullptr;
        uint32_t _queue_family_index = 0;

        VkQueue _compute_queue = nullptr;
        uint32_t _compute_queue_family_index = 0;
        VkCommandPool _compute_command_pool = nullptr;

        VkQueue _transfer_queue = nullptr;
        uint32_t _transfer_queue_family_index = 0;
        VkCommandPool _transfer_command_pool = nullptr;

        // Resource Related Handle

        VkUtil::GraphicsPipeline _pipeline = {};

        std::unique_ptr<ResourceManager> _resource_manager;
        std::unique_ptr<ResourceUploader> _resource_uploader;

        // Frame Resources
        VkDescriptorSetLayout _descriptor_set_layout_frame = nullptr;
        std::array<FrameResource, 2> _frame_resource = {};
        size_t _active_frame_resource_index = 0;

        // Object Resources
        VkDescriptorSetLayout _descriptor_set_layout_object = nullptr;
        std::unordered_map<IdType, ObjectResource> _object_resource;

        std::optional<std::reference_wrapper<const Scene>> _scene = std::nullopt;
    };

} // namespace Vkxel

#endif // VKXEL_RENDERER_H
