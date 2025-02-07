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

        virtual void Create() {}
        virtual void Update() {}
        virtual void Destroy() {}

        virtual ~Object() = default;
    };

} // namespace Vkxel

#endif // VKXEL_OBJECT_H
