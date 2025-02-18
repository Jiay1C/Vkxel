//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_OBJECT_H
#define VKXEL_OBJECT_H

#include <string>
#include <typeinfo>

#include "engine/data_type.h"

namespace Vkxel {

    class Object {
    public:
        std::string name;
        IdType id;

        Object() { id = ObjectCount++; }
        Object(const Object &) = delete;
        Object &operator=(const Object &) = delete;

        Object(Object &&) = default;
        Object &operator=(Object &&) = default;

        // Event Function, Do Not Call Manually
        // Call when create object
        virtual void Init() { name = typeid(*this).name(); }

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

} // namespace Vkxel

#endif // VKXEL_OBJECT_H
