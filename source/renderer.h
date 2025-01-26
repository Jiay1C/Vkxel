//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_RENDERER_H
#define VKXEL_RENDERER_H

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "window.h"
#include "model.h"
#include "camera.h"

namespace Vkxel {

class Renderer {
public:
    void Init();
    void Destroy();

    void AllocateResource();
    void ReleaseResource();
    void UploadData();

    bool Render();

    Window& GetWindow();
    Camera& GetCamera();

private:
    Window _window;
    Camera _camera;

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


    // Resource Related Handle

    VkDescriptorSetLayout _descriptor_set_layout = nullptr;
    VkDescriptorPool _descriptor_pool = nullptr;
    VkDescriptorSet _descriptor_set = nullptr;

    VkPipelineLayout _pipeline_layout = nullptr;
    VkPipeline _pipeline = nullptr;

    VkBuffer _staging_buffer = nullptr;
    VmaAllocation _staging_buffer_allocation = nullptr;
    std::byte* _staging_buffer_pointer = nullptr;

    VkBuffer _vertex_buffer = nullptr;
    VmaAllocation _vertex_buffer_allocation = nullptr;

    VkBuffer _index_buffer = nullptr;
    VmaAllocation _index_buffer_allocation = nullptr;

    VkBuffer _constant_buffer_per_frame = nullptr;
    VmaAllocation _constant_buffer_per_frame_allocation = nullptr;

    VkImage _depth_image = nullptr;
    VkImageView _depth_image_view = nullptr;
    VmaAllocation _depth_image_allocation = nullptr;

    VkSemaphore _image_ready_semaphore = nullptr;
    VkSemaphore _render_complete_semaphore = nullptr;

    VkFence _command_buffer_fence = nullptr;

    // Temp Variable
    const decltype(ModelLibrary::StanfordBunny)& _model = ModelLibrary::StanfordBunny;
};

} // Vkxel

#endif //VKXEL_RENDERER_H
