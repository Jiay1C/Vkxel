//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_CONTROLLER_H
#define VKXEL_CONTROLLER_H

#include "glm/glm.hpp"

#include "transform.h"

namespace Vkxel {

class Controller {
public:
    explicit Controller(Transform& transform);

    Controller& SetMoveSpeed(float speed);
    Controller& SetRotateSpeed(float speed);

    void Update();

private:
    Transform& _transform;
    float _move_speed = 0;
    float _rotate_speed = 0;
    glm::vec2 _last_mouse_position = {};
};

} // Vkxel

#endif //VKXEL_CONTROLLER_H
