//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_OBJECT_H
#define VKXEL_OBJECT_H

#include <string>

#include "nameof.hpp"

#include "engine/data_type.h"
#include "reflect/reflect.hpp"

namespace Vkxel {

    class Object {
    public:
        IdType id;
        std::string name;

        Object() { id = ObjectCount++; }
        Object(const Object &) = delete;
        Object &operator=(const Object &) = delete;

        Object(Object &&) = default;
        Object &operator=(Object &&) = default;

        // Event Function, Do Not Call Manually
        // Call when create object
        virtual void Init() { name = NAMEOF_SHORT_TYPE_RTTI(*this); }

        // Event Function, Do Not Call Manually
        // Call when create scene
        virtual void Create() {}

        // Event Function, Do Not Call Manually
        // Call before the first frame
        virtual void Start() {}

        // Event Function, Do Not Call Manually
        // Call in every frame
        virtual void Update() {}

        // Event Function, Do Not Call Manually
        // Call when destroy the object
        virtual void Destroy() {}

        virtual ~Object() = default;

    private:
        inline static IdType ObjectCount = 0;
    };

    REGISTER_CLASS(Object)
    REGISTER_DATA(id)
    REGISTER_DATA(name)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_OBJECT_H
