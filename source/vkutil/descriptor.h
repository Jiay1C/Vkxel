//
// Created by jiayi on 2/7/2025.
//

#ifndef VKXEL_DESCRIPTOR_H
#define VKXEL_DESCRIPTOR_H

#include "vulkan/vulkan.h"

namespace Vkxel::VkUtil {

    struct DescriptorSet {
        VkDevice device = nullptr;
        VkDescriptorSet set = nullptr;
        VkDescriptorSetLayout layout = nullptr;
        VkDescriptorPool pool = nullptr;

        void Create();
        void Destroy();
    };

    class DescriptorSetBuilder {
    public:
        DescriptorSetBuilder(VkDevice device, VkDescriptorPool descriptorPool,
                             VkDescriptorSetLayout descriptorSetLayout) :
            _device(device), _descriptor_pool(descriptorPool), _descriptor_set_layout(descriptorSetLayout) {}

        DescriptorSet Build() {
            return {.device = _device, .set = nullptr, .layout = _descriptor_set_layout, .pool = _descriptor_pool};
        }

    private:
        VkDevice _device = nullptr;
        VkDescriptorPool _descriptor_pool = nullptr;
        VkDescriptorSetLayout _descriptor_set_layout = nullptr;
    };

} // namespace Vkxel::VkUtil

#endif // VKXEL_DESCRIPTOR_H
