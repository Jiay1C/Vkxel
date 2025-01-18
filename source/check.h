//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_CHECK_H
#define VKXEL_CHECK_H

#include "vulkan/vk_enum_string_helper.h"
#include <cassert>
#include <iostream>

#define CHECK_NOTNULL(f)																				               \
{																										               \
    if (!f)																				                               \
    {																									               \
        std::cerr << "Error : nullptr in " << __FILE__ << " at line " << __LINE__ << std::endl;                        \
        abort();																		                               \
    }																									               \
}

#define CHECK_RESULT(success,f)																				           \
{																										               \
    auto res = (f);																					                   \
    if (res != success)																				                   \
    {																									               \
        std::cerr << "Error : " << res << " in " << __FILE__ << " at line " << __LINE__ << std::endl;                  \
        abort();																		                               \
    }																									               \
}

#define CHECK_RESULT_VK(f)																				               \
{																										               \
    VkResult res = (f);																					               \
    if (res != VK_SUCCESS)																				               \
    {																									               \
        std::cerr << "Error : " << string_VkResult(res) << " in " << __FILE__ << " at line " << __LINE__ << std::endl; \
        abort();																		                               \
    }																									               \
}

#endif //VKXEL_CHECK_H
