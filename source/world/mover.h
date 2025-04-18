//
// Created by jiayi on 2/16/2025.
//

#ifndef VKXEL_MOVER_H
#define VKXEL_MOVER_H

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "component.h"

namespace Vkxel {

    class Mover final : public Component {
    public:
        using Component::Component;

        glm::vec3 linearVelocity = glm::vec3{0, 0, 0};
        glm::vec3 angularVelocity = glm::vec3{0, 0, 0};

        // TODO: Support Space Type Enum(Local, Relative, World)

        void Update() override;
    };

    REGISTER_TYPE(Mover)
    REGISTER_BASE(Component)
    REGISTER_DATA(linearVelocity)
    REGISTER_DATA(angularVelocity)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_MOVER_H
