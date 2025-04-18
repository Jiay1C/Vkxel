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

#define INTERNAL_TYPE(TYPE) VKXEL_INTERNAL_TYPE_##TYPE

#define REGISTER_NAME(NAME)                                                                                            \
    {                                                                                                                  \
        Reflect::SetName(#NAME##_hs, #NAME);                                                                           \
    }

#define REGISTER_BASE(BASE) meta.base<BASE>();

#define REGISTER_DATA(DATA)                                                                                            \
    meta.data<&Type::DATA, entt::as_ref_t>(#DATA##_hs);                                                                \
    REGISTER_NAME(DATA);

#define REGISTER_ENUM(ENUM)                                                                                            \
    meta.data<Type::ENUM>(#ENUM##_hs);                                                                                 \
    REGISTER_NAME(ENUM);

#define REGISTER_TYPE(TYPE)                                                                                            \
    class INTERNAL_TYPE(TYPE) {                                                                                        \
    public:                                                                                                            \
        INTERNAL_TYPE(TYPE)() = delete;                                                                                \
        ~INTERNAL_TYPE(TYPE)() = delete;                                                                               \
        friend class Vkxel::Reflect;                                                                                   \
                                                                                                                       \
    private:                                                                                                           \
        using Type = TYPE;                                                                                             \
        inline static bool CallRegister = []() {                                                                       \
            Reflect::RegisterType<INTERNAL_TYPE(TYPE)>();                                                              \
            return true;                                                                                               \
        }();                                                                                                           \
        static void Register() {                                                                                       \
            using entt::literals::operator""_hs;                                                                       \
            auto meta = entt::meta<Type>();                                                                            \
            REGISTER_NAME(TYPE);

#define REGISTER_END()                                                                                                 \
    }                                                                                                                  \
    }                                                                                                                  \
    ;

namespace Vkxel {

    class Reflect {
    public:
        Reflect() = delete;
        ~Reflect() = delete;

        template<typename InternalType>
        static void RegisterType() {
            static_assert(requires { InternalType::Register; }, "Type Not Registered");
            using Type = typename InternalType::Type;
            if (!_type_map.contains(typeid(Type))) {
                InternalType::Register();
                _type_map[typeid(Type)] = entt::resolve<Type>();
            }
        }

        template<typename T>
        static entt::meta_type GetType() {
            return entt::resolve<T>();
        }

        static const entt::meta_type &GetType(const std::type_index &type) {
            CHECK(_type_map.contains(type), "Type Not Registered: {}", type.name());
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

    private:
        inline static std::unordered_map<std::type_index, entt::meta_type> _type_map;
        inline static std::unordered_map<entt::id_type, std::string_view> _name_map;
    };

} // namespace Vkxel

#endif // VKXEL_REFLECTION_HPP
