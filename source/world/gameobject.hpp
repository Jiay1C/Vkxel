//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_GAMEOBJECT_H
#define VKXEL_GAMEOBJECT_H


#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include "component.h"
#include "engine/timer.h"
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

        void Init() override { transform.Init(); }

        // Event Function, Do Not Call Manually
        void Create() override {
            transform.Create();
            for (const auto &component: _components | std::views::values) {
                component->Create();
            }
        }

        // Event Function, Do Not Call Manually
        void Start() override {
            transform.Create();
            for (const auto &component: _components | std::views::values) {
                component->Start();
            }
        }

        // Event Function, Do Not Call Manually
        void Update() override {
            transform.Update();
            for (const auto &component: _components | std::views::values) {
                component->Update();
            }
        }

        // Event Function, Do Not Call Manually
        void Destroy() override {
            transform.Destroy();
            for (const auto &component: _components | std::views::values) {
                component->Destroy();
            }
            _components.clear();
        }

        template<typename T>
        T &AddComponent() {
            static_assert(std::is_base_of_v<Component, T>, "Type must be derived from Component");

            CHECK(!GetComponent<T>(), "Only support one instance for each Type");

            auto component = std::make_unique<T>(*this);
            T &ref = *component;
            ref.Init();
            _components[Reflect::GetType<T>().id()] = std::move(component);

            return ref;
        }

        Component &AddComponent(const Reflect::Type &type) {
            CHECK(type, "Type must be valid");
            CHECK(type.can_convert(Reflect::GetType<Component>()), "Type must be derived from Component");
            CHECK(type != Reflect::GetType<Transform>(), "Transform is a built-in component");
            CHECK(!GetComponent(type), "Only support one instance for each Type");

            auto component = ComponentFactory::Create(type, *this);
            Component &ref = *component;
            ref.Init();
            _components[type.id()] = std::move(component);

            return ref;
        }

        template<typename T>
        std::optional<std::reference_wrapper<T>> GetComponent() const {
            auto typeId = Reflect::GetType<T>().id();
            if (_components.contains(typeId)) {
                const auto &component = _components.at(typeId);
                if (T *casted_component = dynamic_cast<T *>(component.get())) {
                    return *casted_component;
                }
            }

            return std::nullopt;
        }

        std::optional<std::reference_wrapper<Component>> GetComponent(const Reflect::Type &type) const {
            for (const auto &component: _components | std::views::values) {
                if (Reflect::GetType(typeid(*component)).id() == type.id()) {
                    return *component;
                }
            }

            return std::nullopt;
        }

        std::optional<std::reference_wrapper<Component>> GetComponent(const IdType componentId) const {
            for (auto &component: _components | std::views::values) {
                if (component->id == componentId) {
                    return *component;
                }
            }

            return std::nullopt;
        }

        std::optional<std::reference_wrapper<Component>> GetComponent(const std::string_view componentName) const {
            for (auto &component: _components | std::views::values) {
                if (component->name == componentName) {
                    return *component;
                }
            }

            return std::nullopt;
        }

        auto GetComponentsView() {
            return std::views::all(_components) |
                   std::views::transform([](auto &pair) -> auto & { return pair.second; });
        }

        template<typename T>
        void RemoveComponent() {
            auto typeId = Reflect::GetType<T>().id();
            if (std::type_index type = typeid(T); _components.contains(typeId)) {
                Timer::ExecuteAfterTicks(1, [this, typeId]() {
                    _components.at(typeId)->Destroy();
                    _components.erase(typeId);
                });
            }
        }

        void RemoveComponent(const Component &component) {
            if (auto it = std::ranges::find_if(_components,
                                               [&](const auto &comp) { return comp.second.get() == &component; });
                it != _components.end()) {
                Timer::ExecuteAfterTicks(1, [this, it]() {
                    it->second->Destroy();
                    _components.erase(it);
                });
            }
        }

        void RemoveComponent(const IdType componentId) {
            if (auto it = std::ranges::find_if(_components,
                                               [&](const auto &comp) { return comp.second->id == componentId; });
                it != _components.end()) {
                Timer::ExecuteAfterTicks(1, [this, it]() {
                    it->second->Destroy();
                    _components.erase(it);
                });
            }
        }

        void RemoveComponent(const std::string_view componentName) {
            if (auto it = std::ranges::find_if(_components,
                                               [&](const auto &comp) { return comp.second->name == componentName; });
                it != _components.end()) {
                Timer::ExecuteAfterTicks(1, [this, it]() {
                    it->second->Destroy();
                    _components.erase(it);
                });
            }
        }

    private:
        std::unordered_map<Reflect::ID, std::unique_ptr<Component>> _components;
    };

    REGISTER_TYPE(GameObject)
    REGISTER_BASE(Object)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_GAMEOBJECT_H
