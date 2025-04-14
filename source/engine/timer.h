//
// Created by jiayi on 2/22/2025.
//

#ifndef VKXEL_TIMER_H
#define VKXEL_TIMER_H

#include <queue>

#include "data_type.h"
#include "util/delegate.hpp"

namespace Vkxel {

    class Timer {
    public:
        Timer() = delete;
        ~Timer() = delete;

        static void Update();
        static void ExecuteAfterTicks(TickType tick, const Action &action);
        static void ExecuteAfterSeconds(float second, const Action &action);
        static void ImmediateExecute();

    private:
        using TickActionType = std::pair<TickType, Action>;
        using TimeActionType = std::pair<float, Action>;

        inline static TickType _current_ticks;
        inline static float _current_seconds;

        inline static auto TickActionTypeComparator = [](const TickActionType &a, const TickActionType &b) {
            return a.first > b.first;
        };

        inline static auto TimeActionTypeComparator = [](const TimeActionType &a, const TimeActionType &b) {
            return a.first > b.first;
        };

        inline static std::priority_queue<TickActionType, std::vector<TickActionType>,
                                          decltype(TickActionTypeComparator)>
                _tickAction;
        inline static std::priority_queue<TimeActionType, std::vector<TimeActionType>,
                                          decltype(TimeActionTypeComparator)>
                _timeAction;
    };

} // namespace Vkxel

#endif // VKXEL_TIMER_H
