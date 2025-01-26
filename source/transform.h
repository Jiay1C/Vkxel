//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_TRANSFORM_H
#define VKXEL_TRANSFORM_H

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Vkxel {

class Transform {
public:
    glm::vec3 GetPosition() const;
    void SetPosition(const glm::vec3& position);
    glm::vec3 GetRotation() const;
    void SetRotation(const glm::vec3& rotation);
    glm::vec3 GetScale() const;
    void SetScale(const glm::vec3& scale);

    void TranslateWorld(const glm::vec3& translation);
    void TranslateSelf(const glm::vec3& translation);
    void RotateWorld(const glm::vec3& eulers);
    void RotateSelf(const glm::vec3& eulers);
    void LookAt(const glm::vec3& point, const glm::vec3& up = {0, 1, 0});

    glm::vec3 GetForwardVector() const;
    glm::vec3 GetRightVector() const;
    glm::vec3 GetUpVector() const;

    glm::mat4 GetLocalToWorldMatrix() const;
    glm::mat4 GetWorldToLocalMatrix() const;

private:
    glm::vec3 _position = glm::vec3{0, 0, 0};
    glm::quat _rotation = glm::vec3{0, 0, 0};
    glm::vec3 _scale = glm::vec3{1, 1, 1};
};

} // Vkxel

#endif //VKXEL_TRANSFORM_H
