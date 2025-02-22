//
// Created by jiayi on 1/26/2025.
//

#include "vtime.h"

namespace Vkxel {

    float Time::timeRatio = 1.0f;
    uint64_t Time::_tick_count = 0;

    std::chrono::time_point<std::chrono::steady_clock> Time::_start_timestamp =
            std::chrono::high_resolution_clock::now();

    std::chrono::time_point<std::chrono::steady_clock> Time::_last_update_timestamp = _start_timestamp;
    std::chrono::duration<float> Time::_last_update_duration = {};

    void Time::Update() {
        auto timestamp = std::chrono::high_resolution_clock::now();
        _last_update_duration = timestamp - _last_update_timestamp;
        _last_update_timestamp = timestamp;
        ++_tick_count;
    }

    TickType Time::GetTicks() { return _tick_count; }

    float Time::GetSeconds() {
        auto timestamp = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = timestamp - _start_timestamp;
        return duration.count();
    }


    float Time::GetDeltaSeconds() { return timeRatio * GetRealDeltaSeconds(); }

    float Time::GetRealDeltaSeconds() { return _last_update_duration.count(); }

    void Time::Start() {
        is_running = true;
        _start_time = std::chrono::high_resolution_clock::now();
    }

    void Time::Stop() {
        if (!is_running) {
            return;
        }

        is_running = false;
        auto stop_time = std::chrono::high_resolution_clock::now();
        auto this_duration = stop_time - _start_time;
        _duration += this_duration;
    }

    void Time::Reset() {
        is_running = false;
        _duration = {};
    }

    float Time::GetElapsedSeconds() const { return timeRatio * GetRealElapsedSeconds(); }

    float Time::GetRealElapsedSeconds() const {
        if (is_running) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto this_duration = current_time - _start_time;
            return (_duration + this_duration).count();
        }

        return _duration.count();
    }


} // namespace Vkxel
