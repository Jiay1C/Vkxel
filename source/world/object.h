//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_OBJECT_H
#define VKXEL_OBJECT_H

#include <string>

namespace Vkxel {

    class Object {
    public:
        std::string name;

        Object() = default;
        Object(const Object &) = delete;
        Object &operator=(const Object &) = delete;

        Object(Object &&) = default;
        Object &operator=(Object &&) = default;

        // Event Function
        // Call when create object
        virtual void Init() {}

        // Event Function
        // Call when create scene
        virtual void Create() {}

        // Event Function
        // Call before the first frame
        virtual void Start() {}

        // Event Function
        // Call in every frame
        virtual void Update() {}

        // Event Function
        // Call when destroy the object
        virtual void Destroy() {}

        virtual ~Object() = default;
    };

} // namespace Vkxel

#endif // VKXEL_OBJECT_H
