//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_RENDERER_H
#define VKXEL_RENDERER_H

#include "vulkan/vulkan.h"
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#include "window.h"
#include "model.h"

namespace Vkxel {

class Renderer {
public:
    void Init();
    void Destroy();

    void Allocate();
    void Release();

    void Upload();

    bool Render();

private:
    Window _window;

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

    VkPipelineLayout _pipeline_layout = nullptr;
    VkPipeline _pipeline = nullptr;

    VkBuffer _staging_buffer = nullptr;
    VmaAllocation _staging_buffer_allocation = nullptr;
    uint8_t* _staging_buffer_pointer = nullptr;

    VkBuffer _vertex_buffer = nullptr;
    VmaAllocation _vertex_buffer_allocation = nullptr;

    VkBuffer _index_buffer = nullptr;
    VmaAllocation _index_buffer_allocation = nullptr;

    VkSemaphore _image_ready_semaphore = nullptr;
    VkSemaphore _render_complete_semaphore = nullptr;

    VkFence _command_buffer_fence = nullptr;

    // Temp Variable
    const decltype(ModelLibrary::Triangle)& _model = ModelLibrary::Triangle;
};

} // Vkxel

#endif //VKXEL_RENDERER_H
