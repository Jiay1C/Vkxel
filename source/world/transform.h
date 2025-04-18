//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_TRANSFORM_H
#define VKXEL_TRANSFORM_H

#include <list>
#include <optional>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "component.h"

namespace Vkxel {

    class Transform final : public Component {
    public:
        using Component::Component;

        // Relative Position
        glm::vec3 position = glm::vec3{0, 0, 0};
        glm::quat rotation = glm::vec3{0, 0, 0};
        glm::vec3 scale = glm::vec3{1, 1, 1};

        std::vector<std::reference_wrapper<Transform>> GetChildren() const;
        std::optional<std::reference_wrapper<Transform>> GetParent() const;
        void SetParent(std::optional<std::reference_wrapper<Transform>> parent);

        // Absolute Position
        glm::vec3 GetWorldPosition() const;
        glm::quat GetWorldRotation() const;
        glm::vec3 GetWorldScale() const;

        void SetWorldPosition(const glm::vec3 &worldPosition);
        void SetWorldRotation(const glm::quat &worldRotation);
        void SetWorldScale(const glm::vec3 &worldScale);

        void TranslateWorld(const glm::vec3 &worldTranslation);
        void TranslateRelative(const glm::vec3 &relativeTranslation);
        void TranslateSelf(const glm::vec3 &selfTranslation);

        void RotateWorld(const glm::quat &worldRotation);
        void RotateRelative(const glm::quat &relativeRotation);
        void RotateSelf(const glm::quat &selfRotation);

        glm::vec3 GetForwardVector() const;
        glm::vec3 GetRightVector() const;
        glm::vec3 GetUpVector() const;

        glm::mat4 GetLocalToRelativeMatrix() const;
        glm::mat4 GetRelativeToLocalMatrix() const;

        glm::mat4 GetLocalToWorldMatrix() const;
        glm::mat4 GetWorldToLocalMatrix() const;

        static constexpr glm::vec3 forward{0, 0, -1};
        static constexpr glm::vec3 back{0, 0, 1};
        static constexpr glm::vec3 up{0, 1, 0};
        static constexpr glm::vec3 down{0, -1, 0};
        static constexpr glm::vec3 right{1, 0, 0};
        static constexpr glm::vec3 left{-1, 0, 0};

    private:
        std::optional<std::reference_wrapper<Transform>> _parent;
        std::list<std::reference_wrapper<Transform>> _children;
    };

    REGISTER_TYPE(Transform)
    REGISTER_BASE(Component)
    REGISTER_DATA(position)
    REGISTER_DATA(rotation)
    REGISTER_DATA(scale)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_TRANSFORM_H
