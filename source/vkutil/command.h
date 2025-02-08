//
// Created by jiayi on 2/7/2025.
//

#ifndef VKXEL_COMMAND_H
#define VKXEL_COMMAND_H

#include <functional>

#include "vulkan/vulkan.h"

namespace Vkxel::VkUtil {

    class ImmediateCommand {
    public:
        ImmediateCommand(const VkDevice device, const VkQueue queue, const VkCommandPool commandPool) :
            _device(device), _queue(queue), _command_pool(commandPool) {}

        VkCommandBuffer Begin();
        void End();

        void Run(const std::function<void(VkCommandBuffer)> &command);

    private:
        VkDevice _device = nullptr;
        VkQueue _queue = nullptr;
        VkCommandPool _command_pool = nullptr;

        VkCommandBuffer _command_buffer = nullptr;
        VkFence _fence = nullptr;

        bool _is_recording = false;
    };

} // namespace Vkxel::VkUtil

#endif // VKXEL_COMMAND_H
