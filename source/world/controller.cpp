//
// Created by jiayi on 1/26/2025.
//

#include "controller.h"

#include "engine/input.h"
#include "engine/vtime.h"
#include "gameobject.hpp"

namespace Vkxel {

    void Controller::Update() {
        float delta_seconds = Time::DeltaSeconds();

        glm::vec3 translation = {};

        if (Input::GetKey(KeyCode::KEY_W)) {
            translation += Transform::forward;
        }

        if (Input::GetKey(KeyCode::KEY_S)) {
            translation += Transform::back;
        }

        if (Input::GetKey(KeyCode::KEY_D)) {
            translation += Transform::right;
        }

        if (Input::GetKey(KeyCode::KEY_A)) {
            translation += Transform::left;
        }

        if (Input::GetKey(KeyCode::KEY_E)) {
            translation += Transform::up;
        }

        if (Input::GetKey(KeyCode::KEY_Q)) {
            translation += Transform::down;
        }

        if (Input::GetKey(KeyCode::KEY_LEFT_SHIFT)) {
            translation *= accelerateRatio;
        }

        translation *= moveSpeed * delta_seconds;
        gameObject.transform.TranslateSelf(translation);

        glm::vec2 mouse_position = Input::GetMousePosition();
        glm::vec2 mouse_position_delta = mouse_position - _last_mouse_position;
        _last_mouse_position = mouse_position;

        // No need to add deltaTime since mouse position already include that info
        glm::vec2 rotation = rotateSpeed * mouse_position_delta;

        if (Input::GetKey(KeyCode::MOUSE_BUTTON_RIGHT)) {
            gameObject.transform.RotateSelf({-rotation.y, -rotation.x, 0});
        }
    }

} // namespace Vkxel
