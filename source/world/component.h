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
        using FactoryFunction = std::function<std::unique_ptr<Component>(GameObject &gameObject)>;

        ComponentFactory() = delete;
        ~ComponentFactory() = delete;

        static std::unique_ptr<Component> Create(const Reflect::Type &type, GameObject &gameObject) {
            CHECK(factoryMap.contains(type.id()), "Factory Not Registered For Type: {}", type.info().name());
            return factoryMap.at(type.id())(gameObject);
        }

        template<typename T>
            requires std::derived_from<T, Component>
        static void Register() {
            factoryMap[Reflect::GetType<T>().id()] = [](GameObject &gameObject) {
                return std::make_unique<T>(gameObject);
            };
        }

    private:
        inline static std::unordered_map<Reflect::ID, FactoryFunction> factoryMap;
    };

    template<typename T>
        requires std::derived_from<T, Component>
    struct ReflectPlugin<T> {
        static void Apply() { ComponentFactory::Register<T>(); }
    };

    // Due to static initialization order
    // Must use those macros after Component Factory
    REGISTER_TYPE(Component)
    REGISTER_BASE(Object)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_COMPONENT_H
