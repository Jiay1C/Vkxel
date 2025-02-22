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

        Delegate &Add(const Delegate<Args...> &delegate) {
            _callbacks.insert(_callbacks.end(), delegate._callbacks.begin(), delegate._callbacks.end());
            return *this;
        }

        Delegate &operator+=(const Callback &callback) { return Add(callback); }

        Delegate &operator+=(const Delegate<Args...> &delegate) { return Add(delegate); }

        void Invoke(Args... args) const {
            for (const auto &callback: _callbacks) {
                if (callback)
                    callback(std::forward<Args>(args)...);
            }
        }

        void operator()(Args... args) const { Invoke(std::forward<Args>(args)...); }

        Delegate &Clear() {
            _callbacks.clear();
            return *this;
        }


    private:
        std::vector<Callback> _callbacks;
    };

    using Action = Delegate<>::Callback;

} // namespace Vkxel

#endif // VKXEL_DELEGATE_H
