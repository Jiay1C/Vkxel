//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_CONTROLLER_H
#define VKXEL_CONTROLLER_H

#include "glm/glm.hpp"

#include "transform.h"
#include "util/application.h"

namespace Vkxel {

    class Controller final : public Component {
    public:
        using Component::Component;

        float moveSpeed = Application::DefaultMoveSpeed;
        float rotateSpeed = Application::DefaultRotateSpeed;
        float accelerateRatio = Application::DefaultAccelerateRatio;

        void Update() override;

    private:
        glm::vec2 _last_mouse_position = {};

        REGISTER_BEGIN(Controller)
        REGISTER_BASE(Component)
        REGISTER_DATA(moveSpeed)
        REGISTER_DATA(rotateSpeed)
        REGISTER_DATA(accelerateRatio)
        REGISTER_END()
    };

} // namespace Vkxel

#endif // VKXEL_CONTROLLER_H
