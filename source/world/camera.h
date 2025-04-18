//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_CAMERA_H
#define VKXEL_CAMERA_H

#include "component.h"
#include "glm/glm.hpp"

#include "transform.h"
#include "util/application.h"

namespace Vkxel {


    class Camera final : public Component {
    public:
        using Component::Component;

        float nearClipPlane = Application::DefaultNearClipPlane;
        float farClipPlane = Application::DefaultFarClipPlane;
        float fieldOfViewY = Application::DefaultFov;
        float aspect = static_cast<float>(Application::DefaultWindowWidth) / Application::DefaultWindowHeight;

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
    };

    REGISTER_CLASS(Camera)
    REGISTER_BASE(Component)
    REGISTER_DATA(nearClipPlane)
    REGISTER_DATA(farClipPlane)
    REGISTER_DATA(fieldOfViewY)
    REGISTER_DATA(aspect)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_CAMERA_H
