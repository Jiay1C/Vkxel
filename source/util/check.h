//
// Created by jiayi on 1/18/2025.
//

#ifndef VKXEL_CHECK_H
#define VKXEL_CHECK_H

#include <cassert>

#include "spdlog/spdlog.h"
#include "vulkan/vk_enum_string_helper.h"

// Check if pointer or condition is not null/false and print expression string for context.
#define CHECK(f, ...)                                                                                                  \
    {                                                                                                                  \
        if (!(f)) {                                                                                                    \
            spdlog::error("Expression '{}' is null or false.", #f);                                                    \
            spdlog::error("{} at line {}", __FILE__, __LINE__);                                                        \
            __VA_OPT__(spdlog::error(__VA_ARGS__);)                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }

// Check if a function call returns the expected result.
// 'success' is the expected value and 'f' is the expression to evaluate.
#define CHECK_RESULT(success, f, ...)                                                                                  \
    {                                                                                                                  \
        auto res = (f);                                                                                                \
        if (res != success) {                                                                                          \
            spdlog::error("Expression '{}' returned '{}', expected '{}'.", #f, res, success);                          \
            spdlog::error("{} at line {}", __FILE__, __LINE__);                                                        \
            __VA_OPT__(spdlog::error(__VA_ARGS__);)                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }

// Check Vulkan-specific result and print the corresponding string representation.
// Uses string_VkResult to translate the error code.
#define CHECK_RESULT_VK(f, ...)                                                                                        \
    {                                                                                                                  \
        VkResult res = (f);                                                                                            \
        if (res != VK_SUCCESS) {                                                                                       \
            spdlog::error("Expression '{}' returned '{}'.", #f, string_VkResult(res));                                 \
            spdlog::error("{} at line {}", __FILE__, __LINE__);                                                        \
            __VA_OPT__(spdlog::error(__VA_ARGS__);)                                                                    \
            abort();                                                                                                   \
        }                                                                                                              \
    }

#endif // VKXEL_CHECK_H
