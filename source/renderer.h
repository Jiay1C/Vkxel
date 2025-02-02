//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_RENDERER_H
#define VKXEL_RENDERER_H

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "camera.h"
#include "gui.h"
#include "model.h"
#include "vkutil/buffer.h"
#include "vkutil/image.h"
#include "window.h"

namespace Vkxel {

    class Renderer {
    public:
        friend class GUI;

        explicit Renderer(Window &window, Camera &camera, GUI &gui);

        void Init();
        void Destroy();

        void AllocateResource();
        void ReleaseResource();
        void UploadData();

        void Render();

        void Resize();

        Window &GetWindow() const;
        Camera &GetCamera() const;

    private:
        Window &_window;
        Camera &_camera;
        GUI &_gui;

        bool _pause = false;

        // Device Related Handle

        vkb::Instance _instance;
        vkb::PhysicalDevice _physical_device;
        vkb::Device _device;
        vkb::Swapchain _swapchain;

        std::vector<VkImage> _swapchain_image;
        std::vector<VkImageView> _swapchain_image_view;

        VmaAllocator _vma_allocator = nullptr;

        VkSurfaceKHR _surface = nullptr;
        VkQueue _queue = nullptr;
        VkCommandPool _command_pool = nullptr;

        uint32_t _queue_family_index = 0;


        // Resource Related Handle

        VkDescriptorSetLayout _descriptor_set_layout = nullptr;
        VkDescriptorPool _descriptor_pool = nullptr;
        VkDescriptorSet _descriptor_set = nullptr;

        VkPipelineLayout _pipeline_layout = nullptr;
        VkPipeline _pipeline = nullptr;

        VkUtil::Buffer _staging_buffer = {};
        std::byte *_staging_buffer_pointer = nullptr;

        VkUtil::Buffer _index_buffer = {};
        VkUtil::Buffer _vertex_buffer = {};
        VkUtil::Buffer _constant_buffer_per_frame = {};

        // VkBuffer _staging_buffer = nullptr;
        // VmaAllocation _staging_buffer_allocation = nullptr;
        // std::byte *_staging_buffer_pointer = nullptr;
        //
        // VkBuffer _vertex_buffer = nullptr;
        // VmaAllocation _vertex_buffer_allocation = nullptr;
        //
        // VkBuffer _index_buffer = nullptr;
        // VmaAllocation _index_buffer_allocation = nullptr;
        //
        // VkBuffer _constant_buffer_per_frame = nullptr;
        // VmaAllocation _constant_buffer_per_frame_allocation = nullptr;

        VkUtil::Image _depth_image = {};

        VkSemaphore _image_ready_semaphore = nullptr;
        VkSemaphore _render_complete_semaphore = nullptr;

        VkFence _command_buffer_fence = nullptr;

        // Temp Variable
        const decltype(ModelLibrary::StanfordBunny) &_model = ModelLibrary::StanfordBunny;
    };

} // namespace Vkxel

#endif // VKXEL_RENDERER_H
