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

        using ID = entt::id_type;
        using Type = entt::meta_type;

        template<typename InternalType>
        static void RegisterType() {
            static_assert(requires { InternalType::Register; }, "Type Not Registered");
            using TargetType = typename InternalType::Type;
            if (!_type_map.contains(typeid(TargetType))) {
                InternalType::Register();
                auto type = GetType<TargetType>();
                std::type_index index = typeid(TargetType);
                _type_map[index] = type;
                for (auto &&[id, base]: type.base()) {
                    _derived_map[id].push_back(type);
                }
            }
        }

        template<typename T>
        static Type GetType() {
            return entt::resolve<T>();
        }

        static const Type &GetType(const std::type_index &type) {
            CHECK(_type_map.contains(type), "Type Not Registered: {}", type.name());
            return _type_map.at(type);
        }

        static const std::optional<Type> &TryGetType(const std::type_index &type) {
            if (_type_map.contains(type)) {
                return _type_map.at(type);
            }
            return std::nullopt;
        }

        static const std::vector<Type> &GetDerived(const Type &type) {
            CHECK(_derived_map.contains(type.id()), "Derived Class Not Registered: {}", Reflect::GetName(type.id()));
            return _derived_map.at(type.id());
        }

        static const std::optional<std::vector<Type>> &TryGetDerived(const Type &type) {
            if (_derived_map.contains(type.id())) {
                return _derived_map.at(type.id());
            }
            return std::nullopt;
        }

        static void SetName(const ID id, const std::string_view name) { _name_map[id] = name; }

        static std::string_view GetName(const ID id) {
            CHECK(_name_map.contains(id), "Name Not Registered");
            return _name_map.at(id);
        }

    private:
        inline static std::unordered_map<std::type_index, Type> _type_map;
        inline static std::unordered_map<ID, std::vector<Type>> _derived_map;
        inline static std::unordered_map<ID, std::string_view> _name_map;
    };

} // namespace Vkxel

#endif // VKXEL_REFLECTION_HPP
