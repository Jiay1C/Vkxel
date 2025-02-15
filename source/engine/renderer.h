//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_RENDERER_H
#define VKXEL_RENDERER_H

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "gui.h"
#include "resource.h"
#include "resource_type.h"
#include "vkutil/buffer.h"
#include "vkutil/image.h"
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

        Window &GetWindow() const;

    private:
        // ObjectResource UploadObjectResource(VkCommandBuffer commandBuffer, const ObjectData &object);
        // void DestroyObjectResource(ObjectResource &object);

        Window &_window;
        GUI &_gui;

        bool _init = false;
        bool _pause = false;

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

        // Resource Related Handle

        VkPipelineLayout _pipeline_layout = nullptr;
        VkPipeline _pipeline = nullptr;

        std::unique_ptr<ResourceManager> _resource_manager;
        std::unique_ptr<ResourceUploader> _resource_uploader;

        // Frame Resources
        VkDescriptorSetLayout _descriptor_set_layout_frame = nullptr;
        FrameResource _frame_resource = {};

        // Object Resources
        VkDescriptorSetLayout _descriptor_set_layout_object = nullptr;
        std::vector<ObjectResource> _object_resource = {};

        VkSemaphore _image_ready_semaphore = nullptr;
        VkSemaphore _render_complete_semaphore = nullptr;

        VkFence _command_buffer_fence = nullptr;

        std::optional<std::reference_wrapper<const Scene>> _scene = std::nullopt;
    };

} // namespace Vkxel

#endif // VKXEL_RENDERER_H
