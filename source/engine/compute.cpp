//
// Created by jiayi on 4/3/2025.
//

#include "compute.h"
#include "shader.h"
#include "util/check.h"
#include "vkutil/command.h"
#include "vkutil/descriptor.h"

namespace Vkxel {

    void ComputeJob::Init(const std::string_view shaderPath, const std::vector<std::string_view> &shaderKernels,
                          const std::vector<VkDeviceSize> &bufferSize) {
        // TODO: Allow ReInit
        CHECK(!shaderKernels.empty(), "Must Contain At Least 1 Kernel");

        Destroy();

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

        _compute_pipeline.reserve(shaderKernels.size());
        for (const auto &kernel: shaderKernels) {
            _compute_pipeline.push_back(VkUtil::ComputePipelineBuilder(_device)
                                                .SetShader(shader_module)
                                                .SetShaderName(kernel)
                                                .SetPipelineLayout({_descriptor_set_layout})
                                                .Build());
        }

        vkDestroyShaderModule(_device, shader_module, nullptr);
    }

    void ComputeJob::Dispatch(const VkCommandBuffer commandBuffer, const size_t kernel, const glm::uvec3 group) {
        CHECK(_compute_pipeline[kernel].pipeline, "Compute Job Require Init");
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline[kernel].pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _compute_pipeline[kernel].layout, 0, 1,
                                &_descriptor_set.set, 0, nullptr);
        vkCmdDispatch(commandBuffer, group.x, group.y, group.z);
    }

    void ComputeJob::DispatchImmediate(const size_t kernel, const glm::uvec3 group) {
        VkUtil::ImmediateCommand immediate_command(_device, _queue, _command_pool);
        VkCommandBuffer command_buffer = immediate_command.Begin();
        Dispatch(command_buffer, kernel, group);
        immediate_command.End();
    }

    VkUtil::Buffer &ComputeJob::GetBuffer(const size_t index) { return _compute_buffer[index]; }

    std::vector<std::byte> ComputeJob::ReadBuffer(const size_t index, const size_t offset, const size_t size) {
        size_t buffer_size = size;
        if (buffer_size == 0 || (buffer_size + offset) > _compute_buffer[index].createInfo.size) {
            buffer_size = _compute_buffer[index].createInfo.size - offset;
        }
        std::vector<std::byte> data(buffer_size);
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(ptr + offset, buffer_size, data.data());
        _compute_buffer[index].Unmap();
        return data;
    }

    void ComputeJob::WriteBuffer(const size_t index, const std::vector<std::byte> &data, const size_t offset,
                                 const size_t size) {
        size_t buffer_size = size;
        if (buffer_size == 0 || (buffer_size + offset) > _compute_buffer[index].createInfo.size) {
            buffer_size = _compute_buffer[index].createInfo.size - offset;
        }
        CHECK(buffer_size == data.size(), "Buffer Size Not Match");
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(data.data(), buffer_size, ptr + offset);
        _compute_buffer[index].Flush();
        _compute_buffer[index].Unmap();
    }

    void ComputeJob::ReadBuffer(const size_t index, std::byte *buffer, const size_t offset, const size_t size) {
        size_t buffer_size = size;
        if (buffer_size == 0 || (buffer_size + offset) > _compute_buffer[index].createInfo.size) {
            buffer_size = _compute_buffer[index].createInfo.size - offset;
        }
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(ptr + offset, buffer_size, buffer);
        _compute_buffer[index].Unmap();
    }

    void ComputeJob::WriteBuffer(const size_t index, std::byte *buffer, const size_t offset, const size_t size) {
        size_t buffer_size = size;
        if (buffer_size == 0 || (buffer_size + offset) > _compute_buffer[index].createInfo.size) {
            buffer_size = _compute_buffer[index].createInfo.size - offset;
        }
        std::byte *ptr = _compute_buffer[index].Map();
        std::copy_n(buffer, buffer_size, ptr + offset);
        _compute_buffer[index].Flush();
        _compute_buffer[index].Unmap();
    }

    void ComputeJob::Destroy() {
        if (!_compute_pipeline.empty()) {
            for (auto &buffer: _compute_buffer) {
                buffer.Destroy();
            }
            _compute_buffer = {};

            for (auto &pipeline: _compute_pipeline) {
                pipeline.Destroy();
            }
            _compute_pipeline = {};

            _descriptor_set.Destroy();
            vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
        }
    }

    ComputeJob::~ComputeJob() { Destroy(); }


} // namespace Vkxel
