//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_COMPONENT_H
#define VKXEL_COMPONENT_H

#include "object.h"

namespace Vkxel {

    class GameObject;

    class Component : public Object {
    public:
        GameObject &gameObject;

        explicit Component(GameObject &parentGameObject) : gameObject(parentGameObject) {}
    };

    class ComponentFactory final {
    public:
        using FactoryFunction = std::function<Component*(GameObject &gameObject)>;

        ComponentFactory() = delete;
        ~ComponentFactory() = delete;

        static std::unique_ptr<Component> Create(const Reflect::Type& type, GameObject &gameObject) {
            CHECK(factoryMap.contains(type.id()), "Factory Not Registered For Type: {}", type.info().name());
            return std::unique_ptr<Component>(factoryMap.at(type.id())(gameObject));
        }

        template<typename T>
        static void Register() {
            factoryMap[Reflect::GetType<T>().id()] = [](GameObject &gameObject) -> Component* {
                return new T(gameObject);
            };
        }

    private:
        inline static std::unordered_map<Reflect::ID, FactoryFunction> factoryMap;
    };

    REGISTER_TYPE(Component)
    REGISTER_BASE(Object)
    REGISTER_END()

    template <typename T> requires std::derived_from<T, Component>
    struct ReflectPlugin<T> {
        static void Apply() {
            ComponentFactory::Register<T>();
        }
    };

} // namespace Vkxel

#endif // VKXEL_COMPONENT_H
