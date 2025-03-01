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

        REGISTER_BEGIN(Component)
        REGISTER_BASE(Object)
        REGISTER_END()
    };

} // namespace Vkxel

#endif // VKXEL_COMPONENT_H
