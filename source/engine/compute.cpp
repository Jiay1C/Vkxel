//
// Created by jiayi on 4/3/2025.
//

#include "compute.h"
#include "shader.h"
#include "util/check.h"
#include "vkutil/command.h"
#include "vkutil/descriptor.h"

namespace Vkxel {

    void ComputeJob::Init(const std::string_view shaderPath, const std::string_view shaderEntryPoint,
                          const std::vector<VkDeviceSize> &bufferSize) {
        // TODO: Allow ReInit
        CHECK_NOTNULL_MSG(!_compute_pipeline.pipeline, "Already Init");

        std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_binding(bufferSize.size());
        for (uint32_t index = 0; auto &binding: descriptor_set_layout_binding) {
            binding = {.binding = index++,
                       .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       .descriptorCount = 1,
                       .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                       .pImmutableSamplers = nullptr};
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(descriptor_set_layout_binding.size()),
                .pBindings = descriptor_set_layout_binding.data()};

        CHECK_RESULT_VK(vkCreateDescriptorSetLayout(_device, &descriptor_set_layout_create_info, nullptr,
                                                    &_descriptor_set_layout));


        _descriptor_set = VkUtil::DescriptorSetBuilder(_device, _descriptor_pool, _descriptor_set_layout).Build();
        _descriptor_set.Create();

        std::vector<VkDescriptorBufferInfo> descriptor_buffer_info(bufferSize.size());
        std::vector<VkWriteDescriptorSet> descriptor_set_write_info(bufferSize.size());

        VkUtil::BufferBuilder buffer_builder =
                VkUtil::BufferBuilder(_device, _allocator)
                        .SetPQueueFamilyIndices(&_queue_family)
                        .SetAllocationFlags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT)
                        .SetUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO)
                        .SetRequiredFlags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        _compute_buffer.resize(bufferSize.size());
        for (uint32_t index = 0; index < bufferSize.size(); ++index) {
            _compute_buffer[index] = buffer_builder.SetSize(bufferSize[index]).Build();
            _compute_buffer[index].Create();

            descriptor_buffer_info[index] = {
                    .buffer = _compute_buffer[index].buffer, .offset = 0, .range = VK_WHOLE_SIZE};
            descriptor_set_write_info[index] = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = _descriptor_set.set,
                    .dstBinding = index,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .pBufferInfo = &descriptor_buffer_info[index],
            };
        }

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptor_set_write_info.size()),
                               descriptor_set_write_info.data(), 0, nullptr);

        VkShaderModule shader_module = ShaderLoader::Instance().LoadToModule(_device, shaderPath);

        _compute_pipeline = VkUtil::ComputePipelineBuilder(_device)
                                    .SetShader(shader_module)
                                    .SetShaderName(shaderEntryPoint)
                                    .SetPipelineLayout({_descriptor_set_layout})
                                    .Build();

        vkDestroyShaderModule(_device, shader_module, nullptr);
    }

    void ComputeJob::Dispatch(const VkCommandBuffer commandBuffer, const uint32_t x, const uint32_t y,
                              const uint32_t z) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline.pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline.layout, 0, 1,
                                &_descriptor_set.set, 0, nullptr);
        vkCmdDispatch(commandBuffer, x, y, z);
    }

    void ComputeJob::DispatchImmediate(const uint32_t x, const uint32_t y, const uint32_t z) {
        VkUtil::ImmediateCommand immediate_command(_device, _queue, _command_pool);
        VkCommandBuffer command_buffer = immediate_command.Begin();
        Dispatch(command_buffer, x, y, z);
        immediate_command.End();
    }

    VkUtil::Buffer &ComputeJob::GetBuffer(const size_t index) { return _compute_buffer[index]; }

    std::vector<std::byte> ComputeJob::ReadBuffer(const size_t index) {
        size_t buffer_size = _compute_buffer[index].allocationInfo.size;
        std::vector<std::byte> data(buffer_size);
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(ptr, buffer_size, data.data());
        _compute_buffer[index].Unmap();
        return data;
    }

    void ComputeJob::WriteBuffer(const size_t index, const std::vector<std::byte> &data) {
        size_t buffer_size = _compute_buffer[index].allocationInfo.size;
        CHECK_NOTNULL_MSG(buffer_size == data.size(), "Buffer Size Not Match");
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(data.data(), buffer_size, ptr);
        _compute_buffer[index].Flush();
        _compute_buffer[index].Unmap();
    }

    void ComputeJob::ReadBuffer(const size_t index, std::byte *buffer) {
        size_t buffer_size = _compute_buffer[index].allocationInfo.size;
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(ptr, buffer_size, buffer);
        _compute_buffer[index].Unmap();
    }

    void ComputeJob::WriteBuffer(const size_t index, std::byte *buffer) {
        size_t buffer_size = _compute_buffer[index].allocationInfo.size;
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(buffer, buffer_size, ptr);
        _compute_buffer[index].Flush();
        _compute_buffer[index].Unmap();
    }

    ComputeJob::~ComputeJob() {
        // TODO: Remove Sync?
        vkDeviceWaitIdle(_device);

        for (auto &buffer: _compute_buffer) {
            buffer.Destroy();
        }

        _compute_pipeline.Destroy();
        _descriptor_set.Destroy();
        vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
    }


} // namespace Vkxel
