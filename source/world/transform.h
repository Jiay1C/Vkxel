//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_TRANSFORM_H
#define VKXEL_TRANSFORM_H

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "component.h"

namespace Vkxel {

    class Transform final : Component {
    public:
        using Component::Component;

        glm::vec3 position = glm::vec3{0, 0, 0};
        glm::quat rotation = glm::vec3{0, 0, 0};
        glm::vec3 scale = glm::vec3{1, 1, 1};

        void TranslateWorld(const glm::vec3 &translation);
        void TranslateSelf(const glm::vec3 &translation);
        void RotateWorld(const glm::quat &quaternion);
        void RotateWorld(const glm::vec3 &eulerAngel);
        void RotateSelf(const glm::quat &quaternion);
        void RotateSelf(const glm::vec3 &eulerAngel);
        void LookAt(const glm::vec3 &point, const glm::vec3 &up = {0, 1, 0});

        glm::vec3 GetForwardVector() const;
        glm::vec3 GetRightVector() const;
        glm::vec3 GetUpVector() const;

        glm::mat4 GetLocalToWorldMatrix() const;
        glm::mat4 GetWorldToLocalMatrix() const;

        const static glm::vec3 forward;
        const static glm::vec3 back;
        const static glm::vec3 up;
        const static glm::vec3 down;
        const static glm::vec3 right;
        const static glm::vec3 left;
    };

} // namespace Vkxel

#endif // VKXEL_TRANSFORM_H
