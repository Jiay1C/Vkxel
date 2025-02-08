//
// Created by jiayi on 2/7/2025.
//

#include "descriptor.h"
#include "util/check.h"

namespace Vkxel::VkUtil {
    void DescriptorSet::Create() {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{.sType =
                                                                         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                                 .descriptorPool = pool,
                                                                 .descriptorSetCount = 1,
                                                                 .pSetLayouts = &layout};

        CHECK_RESULT_VK(vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &set));
    }

    void DescriptorSet::Destroy() { CHECK_RESULT_VK(vkFreeDescriptorSets(device, pool, 1, &set)); }


} // namespace Vkxel::VkUtil
