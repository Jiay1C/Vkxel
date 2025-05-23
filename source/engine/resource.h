//
// Created by jiayi on 2/7/2025.
//

#ifndef VKXEL_RESOURCE_H
#define VKXEL_RESOURCE_H

#include <vector>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "data_type.h"
#include "resource_type.h"

namespace Vkxel {

    class ResourceUploader {
    public:
        ResourceUploader(const VkDevice device, const uint32_t queueFamily, const VkQueue queue,
                         const VkCommandPool commandPool, const VmaAllocator allocator) :
            _device(device), _queue_family(queueFamily), _queue(queue), _command_pool(commandPool),
            _allocator(allocator) {}

        ResourceUploader &AddObject(const ObjectData &object, ObjectResource &resource);
        void UploadObjects();
        void Upload();

    private:
        VkDevice _device = nullptr;
        uint32_t _queue_family = 0;
        VkQueue _queue = nullptr;
        VkCommandPool _command_pool = nullptr;
        VmaAllocator _allocator = nullptr;

        std::vector<std::pair<std::reference_wrapper<const ObjectData>, std::reference_wrapper<ObjectResource>>>
                _objects;
    };

    class ResourceManager {
    public:
        ResourceManager(const VkDevice device, const uint32_t queueFamily, const VkCommandPool commandPool,
                        const VkDescriptorPool descriptorPool, const VkDescriptorSetLayout descriptorSetLayoutFrame,
                        const VkDescriptorSetLayout descriptorSetLayoutObject, const VmaAllocator allocator) :
            _device(device), _queue_family(queueFamily), _command_pool(commandPool), _descriptor_pool(descriptorPool),
            _allocator(allocator), _descriptor_set_layout_frame(descriptorSetLayoutFrame),
            _descriptor_set_layout_object(descriptorSetLayoutObject) {}

        ObjectResource CreateObjectResource(const ObjectData &object);
        // TODO: Might modify function signature to support more members
        void UpdateObjectResource(VkCommandBuffer commandBuffer, const ObjectData &object,
                                  ObjectResource &objectResource);
        void DestroyObjectResource(ObjectResource &resource);

        FrameResource CreateFrameResource(uint32_t swapchainWidth, uint32_t swapchainHeight);
        // TODO: Might modify function signature to support more members
        void UpdateFrameResource(VkCommandBuffer commandBuffer, const SceneData &scene, FrameResource &frameResource);
        void DestroyFrameResource(FrameResource &resource);

    private:
        VkDevice _device = nullptr;
        uint32_t _queue_family = 0;
        VkCommandPool _command_pool = nullptr;
        VkDescriptorPool _descriptor_pool = nullptr;
        VmaAllocator _allocator = nullptr;
        VkDescriptorSetLayout _descriptor_set_layout_frame = nullptr;
        VkDescriptorSetLayout _descriptor_set_layout_object = nullptr;
    };

} // namespace Vkxel

#endif // VKXEL_RESOURCE_H
