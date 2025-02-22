//
// Created by jiayi on 2/22/2025.
//

#include "timer.h"
#include "vtime.h"

namespace Vkxel {

    void Timer::Update() {
        _current_ticks = Time::GetTicks();
        _current_seconds = Time::GetSeconds();
        while (!_tickAction.empty()) {
            const auto &[ticks, action] = _tickAction.top();
            if (ticks > _current_ticks) {
                break;
            }
            action();
            _tickAction.pop();
        }
        while (!_timeAction.empty()) {
            const auto &[seconds, action] = _timeAction.top();
            if (seconds > _current_seconds) {
                break;
            }
            action();
            _timeAction.pop();
        }
    }

    void Timer::ExecuteAfterTicks(const TickType tick, const Action &action) {
        if (tick <= 0) {
            action();
            return;
        }
        _tickAction.emplace(_current_ticks + tick, action);
    }

    void Timer::ExecuteAfterSeconds(float second, const Action &action) {
        if (second <= 0) {
            action();
            return;
        }
        _timeAction.emplace(_current_seconds + second, action);
    }


} // namespace Vkxel
