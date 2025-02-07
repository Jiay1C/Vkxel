//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_GAMEOBJECT_H
#define VKXEL_GAMEOBJECT_H


#include <list>
#include <memory>
#include <optional>
#include <string_view>

#include "component.h"
#include "engine/check.h"
#include "object.h"
#include "transform.h"

namespace Vkxel {

    class GameObject final : public Object {
    public:
        Transform transform;

        void Create() override {
            for (auto &component: _components) {
                component->Create();
            }
        }

        void Update() override {
            for (auto &component: _components) {
                component->Update();
            }
        }

        void Destroy() override {
            for (auto &component: _components) {
                component->Destroy();
            }
            _components.clear();
        }

        template<typename T>
        T &AddComponent() {
            static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

            auto component = std::make_unique<T>();
            component->gameObject = this;
            T &ref = *component;
            _components.emplace_back(std::move(component));

            return ref;
        }

        template<typename T>
        std::optional<std::reference_wrapper<T>> GetComponent() const {
            for (auto &comp: _components) {
                if (T *casted = dynamic_cast<T *>(comp.get())) {
                    return *casted;
                }
            }

            return std::nullopt;
        }

        template<typename T>
        void RemoveComponent() {
            for (auto it = _components.begin(); it != _components.end(); ++it) {
                if (T *casted = dynamic_cast<T *>((*it).get())) {
                    casted->Destroy();
                    _components.erase(it);
                }
            }
        }

        void RemoveComponent(const std::string_view componentName) {
            for (auto it = _components.begin(); it != _components.end(); ++it) {
                auto &component = **it;
                if (component.name == componentName) {
                    component.Destroy();
                    _components.erase(it);
                }
            }
        }

        std::list<std::unique_ptr<Component>> _components;
    };

} // namespace Vkxel

#endif // VKXEL_IGAMEOBJECT_H
