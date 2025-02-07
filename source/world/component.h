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
        GameObject *gameObject = nullptr;
    };

} // namespace Vkxel

#endif // VKXEL_COMPONENT_H
