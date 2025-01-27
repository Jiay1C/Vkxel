//
// Created by jiayi on 1/26/2025.
//

#include "controller.h"

#include "input.h"
#include "vtime.h"

namespace Vkxel {
     Controller::Controller(Transform &transform) : _transform(transform) {  }

    Controller& Controller::SetMoveSpeed(const float speed) {
        _move_speed = speed;
         return *this;
    }

    Controller& Controller::SetRotateSpeed(const float speed) {
        _rotate_speed = speed;
         return *this;
    }

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

        _transform.TranslateSelf(translation * _move_speed * delta_seconds);

        glm::vec2 mouse_position = Input::GetMousePosition();
        glm::vec2 mouse_position_delta = mouse_position - _last_mouse_position;
        _last_mouse_position = mouse_position;

        // No need to add deltaTime since mouse position already include that info
        glm::vec2 camera_rotation = _rotate_speed * mouse_position_delta;

        if (Input::GetKey(KeyCode::MOUSE_BUTTON_RIGHT)) {
            _transform.RotateSelf({-camera_rotation.y, -camera_rotation.x, 0});
        }
    }

} // Vkxel