//
// Created by jiayi on 2/1/2025.
//

#ifndef VKXEL_DELEGATE_H
#define VKXEL_DELEGATE_H

#include <functional>
#include <vector>

namespace Vkxel {

    template<typename... Args>
    class Delegate {
    public:
        using Callback = std::function<void(Args...)>;

        Delegate &Add(const Callback &callback) {
            _callbacks.push_back(callback);
            return *this;
        }

        Delegate &operator+=(const Callback &callback) { return Add(callback); }

        Delegate &Invoke(Args... args) {
            for (const auto &callback: _callbacks) {
                if (callback)
                    callback(std::forward<Args>(args)...);
            }
            return *this;
        }

        Delegate &operator()(Args... args) { return Invoke(std::forward<Args>(args)...); }

        Delegate &Clear() {
            _callbacks.clear();
            return *this;
        }


    private:
        std::vector<Callback> _callbacks;
    };

} // namespace Vkxel

#endif // VKXEL_DELEGATE_H
