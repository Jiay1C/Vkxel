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
#include "object.h"
#include "transform.h"
#include "util/check.h"

namespace Vkxel {

    class Scene;

    class GameObject final : public Object {
    public:
        Scene &scene;
        Transform transform = Transform(*this);

        explicit GameObject(Scene &parentScene) : scene(parentScene) {}

        // Event Function, Do Not Call Manually
        void Create() override {
            for (auto &component: _components) {
                component->Create();
            }
        }

        // Event Function, Do Not Call Manually
        void Update() override {
            for (auto &component: _components) {
                component->Update();
            }
        }

        // Event Function, Do Not Call Manually
        void Destroy() override {
            for (auto &component: _components) {
                component->Destroy();
            }
            _components.clear();
        }

        template<typename T>
        T &AddComponent() {
            static_assert(std::is_base_of_v<Component, T>, "Type must be derived from Component");

            CHECK_NOTNULL_MSG(!GetComponent<T>(), "Only support one instance for every Type");

            auto component = std::make_unique<T>(*this);
            T &ref = *component;
            ref.Init();
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

        std::list<std::unique_ptr<Component>> &GetComponentList() { return _components; }

        template<typename T>
        void RemoveComponent() {
            std::erase_if(_components, [](std::unique_ptr<Component> &comp) {
                if (T *casted = dynamic_cast<T *>(comp.get())) {
                    casted->Destroy();
                    return true;
                }
                return false;
            });
        }

        void RemoveComponent(const Component &component) {
            std::erase_if(_components, [&component](std::unique_ptr<Component> &comp) {
                if (comp.get() == &component) {
                    comp->Destroy();
                    return true;
                }
                return false;
            });
        }

        void RemoveComponent(const IdType componentId) {
            std::erase_if(_components, [&componentId](std::unique_ptr<Component> &comp) {
                if (comp->id == componentId) {
                    comp->Destroy();
                    return true;
                }
                return false;
            });
        }

        void RemoveComponent(std::string_view componentName) {
            std::erase_if(_components, [&componentName](std::unique_ptr<Component> &comp) {
                if (comp->name == componentName) {
                    comp->Destroy();
                    return true;
                }
                return false;
            });
        }

    private:
        std::list<std::unique_ptr<Component>> _components;
    };

} // namespace Vkxel

#endif // VKXEL_GAMEOBJECT_H
