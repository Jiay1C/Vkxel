#include "glm/glm.hpp"

#include "transform.h"

namespace Vkxel {

    std::vector<std::reference_wrapper<Transform>> Transform::GetChildren() const {
        return {_children.begin(), _children.end()};
    }

    std::optional<std::reference_wrapper<Transform>> Transform::GetParent() const { return _parent; }

    void Transform::SetParent(std::optional<std::reference_wrapper<Transform>> newParent) {
        if (_parent) {
            auto &currentParent = _parent.value().get();
            std::erase_if(currentParent._children,
                          [this](const std::reference_wrapper<Transform> &child) { return &child.get() == this; });
        }
        _parent = newParent;
        if (newParent) {
            newParent.value().get()._children.emplace_back(*this);
        }
    }

    glm::vec3 Transform::GetWorldPosition() const {
        return _parent ? _parent->get().GetWorldPosition() + _parent->get().GetWorldRotation() * position : position;
    }

    glm::quat Transform::GetWorldRotation() const {
        return _parent ? _parent->get().GetWorldRotation() * rotation : rotation;
    }

    glm::vec3 Transform::GetWorldScale() const { return _parent ? _parent->get().GetWorldScale() * scale : scale; }

    void Transform::SetWorldPosition(const glm::vec3 &worldPosition) {
        position = _parent ? glm::inverse(_parent->get().GetWorldRotation()) *
                                     (worldPosition - _parent->get().GetWorldPosition())
                           : worldPosition;
    }

    void Transform::SetWorldRotation(const glm::quat &worldRotation) {
        rotation = _parent ? glm::inverse(_parent->get().GetWorldRotation()) * worldRotation : worldRotation;
    }

    void Transform::SetWorldScale(const glm::vec3 &worldScale) {
        scale = _parent ? worldScale / _parent->get().GetWorldScale() : worldScale;
    }

    void Transform::TranslateWorld(const glm::vec3 &worldTranslation) {
        SetWorldPosition(GetWorldPosition() + worldTranslation);
    }

    void Transform::TranslateRelative(const glm::vec3 &relativeTranslation) { position += relativeTranslation; }

    void Transform::TranslateSelf(const glm::vec3 &selfTranslation) { position += rotation * selfTranslation; }

    void Transform::RotateWorld(const glm::quat &worldRotation) {
        SetWorldRotation(worldRotation * GetWorldRotation());
    }

    void Transform::RotateRelative(const glm::quat &relativeRotation) { rotation = relativeRotation * rotation; }

    void Transform::RotateSelf(const glm::quat &selfRotation) { rotation *= selfRotation; }

    glm::vec3 Transform::GetForwardVector() const { return GetWorldRotation() * forward; }

    glm::vec3 Transform::GetRightVector() const { return GetWorldRotation() * right; }

    glm::vec3 Transform::GetUpVector() const { return GetWorldRotation() * up; }

    glm::mat4 Transform::GetLocalToRelativeMatrix() const {
        return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) *
               glm::scale(glm::mat4(1.0f), scale);
    }

    glm::mat4 Transform::GetRelativeToLocalMatrix() const { return glm::inverse(GetLocalToRelativeMatrix()); }

    glm::mat4 Transform::GetLocalToWorldMatrix() const {
        return _parent ? _parent->get().GetLocalToWorldMatrix() * GetLocalToRelativeMatrix()
                       : GetLocalToRelativeMatrix();
    }

    glm::mat4 Transform::GetWorldToLocalMatrix() const { return glm::inverse(GetLocalToWorldMatrix()); }

} // namespace Vkxel
