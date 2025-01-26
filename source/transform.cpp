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

    // Translate in world space
    void Transform::TranslateWorld(const glm::vec3& translation) {
        position += translation;
    }

    // Translate relative to the object's local space
    void Transform::TranslateSelf(const glm::vec3& translation) {
        position += rotation * translation;
    }

    // Rotate in world space
    void Transform::RotateWorld(const glm::vec3& eulerAngel) {
        rotation = glm::quat(eulerAngel) * rotation;
    }

    void Transform::RotateWorld(const glm::quat &quaternion) {
        rotation = quaternion * rotation;
    }

    // Rotate relative to the object's local space
    void Transform::RotateSelf(const glm::vec3& eulerAngel) {
        rotation *= glm::quat(eulerAngel);
    }

    void Transform::RotateSelf(const glm::quat &quaternion) {
        rotation *= quaternion;
    }

    void Transform::LookAt(const glm::vec3 &point, const glm::vec3 &up) {
        glm::vec3 direction = normalize((point - position));
        rotation = glm::quatLookAt(direction, up);
    }


    // Get the forward vector of the transform
    glm::vec3 Transform::GetForwardVector() const {
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    // Get the right vector of the transform
    glm::vec3 Transform::GetRightVector() const {
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Get the up vector of the transform
    glm::vec3 Transform::GetUpVector() const {
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    // Get the local-to-world transformation matrix
    glm::mat4 Transform::GetLocalToWorldMatrix() const {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationMatrix = glm::toMat4(rotation);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    // Get the world-to-local transformation matrix
    glm::mat4 Transform::GetWorldToLocalMatrix() const {
        return glm::inverse(GetLocalToWorldMatrix());
    }


} // Vkxel