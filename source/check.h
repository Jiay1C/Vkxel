//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_CHECK_H
#define VKXEL_CHECK_H

#include <cassert>
#include <iostream>

#include "vulkan/vk_enum_string_helper.h"

#define CHECK_NOTNULL(f)                                                                                               \
    {                                                                                                                  \
        if (!f) {                                                                                                      \
            std::cerr << "Error : nullptr or false in " << __FILE__ << " at line " << __LINE__ << std::endl;           \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#define CHECK_RESULT(success, f)                                                                                       \
    {                                                                                                                  \
        auto res = (f);                                                                                                \
        if (res != success) {                                                                                          \
            std::cerr << "Error : " << res << " in " << __FILE__ << " at line " << __LINE__ << std::endl;              \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#define CHECK_RESULT_VK(f)                                                                                             \
    {                                                                                                                  \
        VkResult res = (f);                                                                                            \
        if (res != VK_SUCCESS) {                                                                                       \
            std::cerr << "Error : " << string_VkResult(res) << " in " << __FILE__ << " at line " << __LINE__           \
                      << std::endl;                                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#define CHECK_NOTNULL_MSG(f, msg)                                                                                      \
    {                                                                                                                  \
        if (!(f)) {                                                                                                    \
            std::cerr << "Error: nullptr or false in " << __FILE__ << " at line " << __LINE__ << ". Msg: " << msg      \
                      << std::endl;                                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#define CHECK_RESULT_MSG(success, f, msg)                                                                              \
    {                                                                                                                  \
        auto res = (f);                                                                                                \
        if (res != success) {                                                                                          \
            std::cerr << "Error: " << res << " in " << __FILE__ << " at line " << __LINE__ << ". Msg: " << msg         \
                      << std::endl;                                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#define CHECK_RESULT_VK_MSG(f, msg)                                                                                    \
    {                                                                                                                  \
        VkResult res = (f);                                                                                            \
        if (res != VK_SUCCESS) {                                                                                       \
            std::cerr << "Error: " << string_VkResult(res) << " in " << __FILE__ << " at line " << __LINE__            \
                      << ". Msg: " << msg << std::endl;                                                                \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#endif // VKXEL_CHECK_H
