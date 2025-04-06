//
// Created by jiayi on 4/3/2025.
//

#ifndef VKXEL_COMPUTE_H
#define VKXEL_COMPUTE_H

#include <string_view>
#include <vector>

#include "glm/glm.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan_core.h"

#include "vkutil/buffer.h"
#include "vkutil/descriptor.h"
#include "vkutil/pipeline.h"

namespace Vkxel {

    class ComputeJob {
    public:
        ComputeJob(const VkDevice device, const uint32_t queueFamily, const VkQueue queue,
                   const VkCommandPool commandPool, const VkDescriptorPool descriptorPool,
                   const VmaAllocator allocator) :
            _device(device), _queue_family(queueFamily), _queue(queue), _command_pool(commandPool),
            _descriptor_pool(descriptorPool), _allocator(allocator) {}

        void Init(std::string_view shaderPath, const std::vector<std::string_view> &shaderKernels,
                  const std::vector<VkDeviceSize> &bufferSize);

        void Dispatch(VkCommandBuffer commandBuffer, size_t kernel, glm::uvec3 group);
        void DispatchImmediate(size_t kernel, glm::uvec3 group);

        VkUtil::Buffer &GetBuffer(size_t index);
        std::vector<std::byte> ReadBuffer(size_t index, size_t offset = 0, size_t size = 0);
        void WriteBuffer(size_t index, const std::vector<std::byte> &data, size_t offset = 0, size_t size = 0);
        void ReadBuffer(size_t index, std::byte *buffer, size_t offset = 0, size_t size = 0);
        void WriteBuffer(size_t index, std::byte *buffer, size_t offset = 0, size_t size = 0);

        void Destroy();

        ~ComputeJob();

    private:
        VkDevice _device = nullptr;
        uint32_t _queue_family = 0;
        VkQueue _queue = nullptr;
        VkCommandPool _command_pool = nullptr;
        VkDescriptorPool _descriptor_pool = nullptr;
        VmaAllocator _allocator = nullptr;

        VkDescriptorSetLayout _descriptor_set_layout = nullptr;
        std::vector<VkUtil::Buffer> _compute_buffer = {};
        std::vector<VkUtil::ComputePipeline> _compute_pipeline = {};
        VkUtil::DescriptorSet _descriptor_set = {};
    };

} // namespace Vkxel

#endif // VKXEL_COMPUTE_H
