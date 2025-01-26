//
// Created by jiayi on 1/26/2025.
//

#include "vtime.h"

namespace Vkxel {

    float Time::timeRatio = 1.0f;
    std::chrono::time_point<std::chrono::steady_clock> Time::_last_update_timestamp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> Time::_last_update_duration = {};

    void Time::Update() {
        auto timestamp = std::chrono::high_resolution_clock::now();
        _last_update_duration = timestamp - _last_update_timestamp;
        _last_update_timestamp = timestamp;
    }

    float Time::DeltaSeconds() {
        return timeRatio * RealDeltaSeconds();
    }

    float Time::RealDeltaSeconds() {
        return _last_update_duration.count();
    }

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

    float Time::ElapsedSeconds() const {
        return timeRatio * RealElapsedSeconds();
    }

    float Time::RealElapsedSeconds() const {
        if (is_running) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto this_duration = current_time - _start_time;
            return (_duration + this_duration).count();
        }

        return _duration.count();
    }


} // Vkxel