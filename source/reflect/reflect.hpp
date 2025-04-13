//
// Created by jiayi on 2/28/2025.
//

#ifndef VKXEL_REFLECTION_HPP
#define VKXEL_REFLECTION_HPP

#include <optional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "entt/entt.hpp"

#include "util/check.h"

#define REGISTER_NAME(NAME) Reflect::SetName(#NAME##_hs, #NAME)

#define REGISTER_BASE(BASE) meta.base<BASE>();

#define REGISTER_DATA(DATA)                                                                                            \
    meta.data<&T::DATA, entt::as_ref_t>(#DATA##_hs);                                                                   \
    REGISTER_NAME(DATA);

#define REGISTER_BEGIN(CLASS)                                                                                          \
public:                                                                                                                \
    friend class Vkxel::Reflect;                                                                                       \
                                                                                                                       \
private:                                                                                                               \
    static void Register() {                                                                                           \
        using T = CLASS;                                                                                               \
        using entt::literals::operator""_hs;                                                                           \
        auto meta = entt::meta<T>();                                                                                   \
        REGISTER_NAME(CLASS);

#define REGISTER_END() }


namespace Vkxel {

    class Reflect {
    public:
        Reflect() = delete;
        ~Reflect() = delete;

        template<typename T>
        static void Register() {
            static_assert(requires { T::Register; }, "Type Not Registered");
            if (!_type_map.contains(typeid(T))) {
                T::Register();
                _type_map[typeid(T)] = entt::resolve<T>();
            }
        }

        template<typename T>
        static entt::meta_type GetType() {
            return entt::resolve<T>();
        }

        static const entt::meta_type &GetType(const std::type_index &type) {
            CHECK(_type_map.contains(type), "Type Not Registered");
            return _type_map.at(type);
        }

        static const std::optional<entt::meta_type> &TryGetType(const std::type_index &type) {
            if (_type_map.contains(type)) {
                return _type_map.at(type);
            }
            return std::nullopt;
        }

        static void SetName(const entt::id_type id, const std::string_view name) { _name_map[id] = name; }

        static std::string_view GetName(const entt::id_type id) {
            CHECK(_name_map.contains(id), "Name Not Registered");
            return _name_map.at(id);
        }

        static void Register();

    private:
        inline static std::unordered_map<std::type_index, entt::meta_type> _type_map;
        inline static std::unordered_map<entt::id_type, std::string_view> _name_map;
    };

} // namespace Vkxel

#endif // VKXEL_REFLECTION_HPP
