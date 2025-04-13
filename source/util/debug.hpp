//
// Created by jiayi on 4/13/2025.
//

#ifndef VKXEL_DEBUG_H
#define VKXEL_DEBUG_H

#include "spdlog/spdlog.h"

namespace Vkxel {

    class Debug {
    public:
        Debug() = delete;
        ~Debug() = delete;

        enum class LogLevel { Silence, Debug, Info, Warning, Error };

        template<typename... Args>
        using Text = spdlog::format_string_t<Args...>;

        template<typename... Args>
        static void Log(Text<Args...> text, Args &&...args) {
            spdlog::debug(text, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void LogInfo(Text<Args...> text, Args &&...args) {
            spdlog::info(text, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void LogWarning(Text<Args...> text, Args &&...args) {
            spdlog::warn(text, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void LogError(Text<Args...> text, Args &&...args) {
            spdlog::error(text, std::forward<Args>(args)...);
        }

        static void SetLogLevel(const LogLevel level) {
            switch (level) {
                case LogLevel::Silence:
                    spdlog::set_level(spdlog::level::off);
                    break;
                case LogLevel::Debug:
                    spdlog::set_level(spdlog::level::debug);
                    break;
                case LogLevel::Info:
                    spdlog::set_level(spdlog::level::info);
                    break;
                case LogLevel::Warning:
                    spdlog::set_level(spdlog::level::warn);
                    break;
                case LogLevel::Error:
                    spdlog::set_level(spdlog::level::err);
                    break;
            }
        }

        static void Init() {
#ifdef NDEBUG
            SetLogLevel(LogLevel::Info);
#else
            SetLogLevel(LogLevel::Debug);
#endif
        }
    };

} // namespace Vkxel

#endif // VKXEL_DEBUG_H
