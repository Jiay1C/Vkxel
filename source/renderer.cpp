//
// Created by jiayi on 1/18/2025.
//

#include "vulkan/vulkan.h"
#include "VkBootstrap.h"

#include "renderer.h"
#include "application.h"
#include "check.h"


namespace Vkxel {
    void Renderer::Init() {

        // Create Instance
        vkb::InstanceBuilder instance_builder;
        auto instance_result = instance_builder
            .set_app_name(Application::Name.data())
            .set_app_version(Application::Version)
            .require_api_version(Application::VulkanVersion)
#ifndef NDEBUG
            .use_default_debug_messenger()
            .request_validation_layers()
#endif
            .build();
        CHECK_NOTNULL_MSG(instance_result, instance_result.error().message());
        _instance = instance_result.value();

        // Create Window
        _window.SetResolution(Application::DefaultResolution.first, Application::DefaultResolution.second)
               .SetTitle(Application::Name)
               .SetInstance(_instance)
               .Create();

        _surface = _window.GetSurface();

        // Select Physical Device
        vkb::PhysicalDeviceSelector physical_device_selector(_instance);
        auto physical_device_result = physical_device_selector
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
            .set_surface(_surface)
            .select();
        CHECK_NOTNULL_MSG(physical_device_result, physical_device_result.error().message());
        _physical_device = physical_device_result.value();

        // Create Device
        vkb::DeviceBuilder device_builder(_physical_device);
        auto device_result = device_builder.build();
        CHECK_NOTNULL_MSG(device_result, device_result.error().message());

        _device = device_result.value();

        // Create Swapchain
        vkb::SwapchainBuilder swapchain_builder(_device);
        auto swapchain_result = swapchain_builder
            .set_old_swapchain(_swapchain)
            .set_desired_present_mode(Application::DefaultPresentMode)
            .build();
        CHECK_NOTNULL_MSG(swapchain_result, swapchain_result.error().message());
        _swapchain = swapchain_result.value();

        // Get Queue
        auto queue_result = _device.get_queue(vkb::QueueType::graphics);
        CHECK_NOTNULL_MSG(queue_result, queue_result.error().message());
        _queue = queue_result.value();


    }

    void Renderer::Destroy() {
        vkb::destroy_swapchain(_swapchain);
        vkb::destroy_device(_device);
        _window.Destroy();
        vkb::destroy_instance(_instance);
    }

    void Renderer::Allocate() {

    }

    void Renderer::Release() {

    }

    bool Renderer::Render() {
        return _window.Update();
    }


} // Vkxel