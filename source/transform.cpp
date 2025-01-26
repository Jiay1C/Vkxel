//
// Created by jiayi on 1/25/2025.
//

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "transform.h"

namespace Vkxel {
    // Getter and Setter for Position
    glm::vec3 Transform::GetPosition() const {
        return _position;
    }

    void Transform::SetPosition(const glm::vec3& position) {
        _position = position;
    }

    // Getter and Setter for Rotation
    glm::vec3 Transform::GetRotation() const {
        return glm::eulerAngles(_rotation);
    }

    void Transform::SetRotation(const glm::vec3& rotation) {
        _rotation = glm::quat(rotation);
    }

    // Getter and Setter for Scale
    glm::vec3 Transform::GetScale() const {
        return _scale;
    }

    void Transform::SetScale(const glm::vec3& scale) {
        _scale = scale;
    }

    // Translate in world space
    void Transform::TranslateWorld(const glm::vec3& translation) {
        _position += translation;
    }

    // Translate relative to the object's local space
    void Transform::TranslateSelf(const glm::vec3& translation) {
        _position += _rotation * translation;
    }

    // Rotate in world space
    void Transform::RotateWorld(const glm::vec3& eulers) {
        glm::quat worldRotation = glm::quat(eulers);
        _rotation = worldRotation * _rotation;
    }

    // Rotate relative to the object's local space
    void Transform::RotateSelf(const glm::vec3& eulers) {
        glm::quat localRotation = glm::quat(eulers);
        _rotation *= localRotation;
    }

    void Transform::LookAt(const glm::vec3 &point, const glm::vec3 &up) {
        glm::vec3 direction = normalize((point - _position));
        _rotation = glm::quatLookAt(direction, up);
    }


    // Get the forward vector of the transform
    glm::vec3 Transform::GetForwardVector() const {
        return _rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    // Get the right vector of the transform
    glm::vec3 Transform::GetRightVector() const {
        return _rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Get the up vector of the transform
    glm::vec3 Transform::GetUpVector() const {
        return _rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    // Get the local-to-world transformation matrix
    glm::mat4 Transform::GetLocalToWorldMatrix() const {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), _position);
        glm::mat4 rotationMatrix = glm::toMat4(_rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), _scale);
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    // Get the world-to-local transformation matrix
    glm::mat4 Transform::GetWorldToLocalMatrix() const {
        return glm::inverse(GetLocalToWorldMatrix());
    }


} // Vkxel