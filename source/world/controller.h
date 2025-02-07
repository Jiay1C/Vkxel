//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_CONTROLLER_H
#define VKXEL_CONTROLLER_H

#include "glm/glm.hpp"

#include "transform.h"

namespace Vkxel {

    class Controller : public Component {
    public:
        Controller &SetMoveSpeed(float speed);
        Controller &SetRotateSpeed(float speed);

        void Update() override;

    private:
        float _move_speed = 0;
        float _rotate_speed = 0;
        glm::vec2 _last_mouse_position = {};
    };

} // namespace Vkxel

#endif // VKXEL_CONTROLLER_H
