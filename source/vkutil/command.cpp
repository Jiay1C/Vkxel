//
// Created by jiayi on 2/7/2025.
//

#include "vulkan/vulkan.h"

#include "command.h"
#include "util/check.h"

namespace Vkxel::VkUtil {

    VkCommandBuffer ImmediateCommand::Begin() {
        CHECK(!_is_recording);
        _is_recording = true;

        VkCommandBufferAllocateInfo command_buffer_allocate_info{.sType =
                                                                         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                                 .commandPool = _command_pool,
                                                                 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                                 .commandBufferCount = 1};

        CHECK_RESULT_VK(vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, &_command_buffer));

        VkCommandBufferBeginInfo command_buffer_begin_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                           .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

        CHECK_RESULT_VK(vkBeginCommandBuffer(_command_buffer, &command_buffer_begin_info));
        return _command_buffer;
    }

    void ImmediateCommand::End() {
        CHECK(_is_recording);
        _is_recording = false;

        CHECK_RESULT_VK(vkEndCommandBuffer(_command_buffer));

        VkFenceCreateInfo fence_create_info{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        CHECK_RESULT_VK(vkCreateFence(_device, &fence_create_info, nullptr, &_fence));

        VkCommandBufferSubmitInfo command_buffer_submit_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                                                             .commandBuffer = _command_buffer};

        VkSubmitInfo2 submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                                  .commandBufferInfoCount = 1,
                                  .pCommandBufferInfos = &command_buffer_submit_info};
        vkQueueSubmit2(_queue, 1, &submit_info, _fence);

        CHECK_RESULT_VK(vkWaitForFences(_device, 1, &_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkDestroyFence(_device, _fence, nullptr);

        vkFreeCommandBuffers(_device, _command_pool, 1, &_command_buffer);
    }

    void ImmediateCommand::Run(const std::function<void(VkCommandBuffer)> &command) {
        Begin();
        command(_command_buffer);
        End();
    }


} // namespace Vkxel::VkUtil
