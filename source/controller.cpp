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

        float camera_translation = _move_speed * delta_seconds;

        if (Input::GetKey(KeyCode::KEY_W)) {
            _transform.TranslateSelf(glm::vec3{0, 0, -1} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_S)) {
            _transform.TranslateSelf(glm::vec3{0, 0, 1} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_D)) {
            _transform.TranslateSelf(glm::vec3{1, 0, 0} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_A)) {
            _transform.TranslateSelf(glm::vec3{-1, 0, 0} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_Q)) {
            _transform.TranslateSelf(glm::vec3{0, -1, 0} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_E)) {
            _transform.TranslateSelf(glm::vec3{0, 1, 0} * camera_translation);
        }

        glm::vec2 mouse_position = Input::GetMousePosition();
        glm::vec2 mouse_position_delta = mouse_position - _last_mouse_position;
        _last_mouse_position = mouse_position;

        glm::vec2 camera_rotation = _rotate_speed * delta_seconds * mouse_position_delta;

        if (Input::GetKey(KeyCode::MOUSE_BUTTON_RIGHT)) {
            _transform.RotateSelf({-camera_rotation.y, -camera_rotation.x, 0});
        }
    }

} // Vkxel