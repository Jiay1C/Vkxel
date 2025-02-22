//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_VTIME_H
#define VKXEL_VTIME_H

#include <chrono>

#include "data_type.h"

namespace Vkxel {

    class Time {
    public:
        static float timeRatio;

        static void Update();

        static TickType GetTicks();
        static float GetSeconds();
        static float GetDeltaSeconds();
        static float GetRealDeltaSeconds();

        void Start();
        void Stop();
        void Reset();

        float GetElapsedSeconds() const;
        float GetRealElapsedSeconds() const;

    private:
        static TickType _tick_count;
        static std::chrono::time_point<std::chrono::steady_clock> _start_timestamp;
        static std::chrono::duration<float> _last_update_duration;
        static std::chrono::time_point<std::chrono::steady_clock> _last_update_timestamp;

        bool is_running = false;
        std::chrono::time_point<std::chrono::steady_clock> _start_time;
        std::chrono::duration<float> _duration = {};
    };

} // namespace Vkxel

#endif // VKXEL_VTIME_H
