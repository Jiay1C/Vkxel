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
        ObjectResource UploadObjectResource(VkCommandBuffer commandBuffer, const ObjectData &object);
        void DestroyObjectResource(ObjectResource &object);

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

        VmaAllocator _vma_allocator = nullptr;

        VkSurfaceKHR _surface = nullptr;
        VkQueue _queue = nullptr;
        VkCommandPool _command_pool = nullptr;
        VkDescriptorPool _descriptor_pool = nullptr;

        uint32_t _queue_family_index = 0;

        // Resource Related Handle

        VkDescriptorSetLayout _descriptor_set_layout = nullptr;

        VkPipelineLayout _pipeline_layout = nullptr;
        VkPipeline _pipeline = nullptr;

        VkUtil::Buffer _staging_buffer = {};
        std::byte *_staging_buffer_pointer = nullptr;
        uint32_t _staging_buffer_count = 0;

        // VkUtil::Buffer _index_buffer = {};
        // VkUtil::Buffer _vertex_buffer = {};
        // VkUtil::Buffer _constant_buffer_per_frame = {};

        FrameResource _frame_resource = {};
        std::vector<ObjectResource> _object_resource = {};

        VkUtil::Image _color_image = {};
        VkUtil::Image _depth_image = {};

        VkSemaphore _image_ready_semaphore = nullptr;
        VkSemaphore _render_complete_semaphore = nullptr;

        VkFence _command_buffer_fence = nullptr;

        std::optional<std::reference_wrapper<const Scene>> _scene = std::nullopt;
    };

} // namespace Vkxel

#endif // VKXEL_RENDERER_H
