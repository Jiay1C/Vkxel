//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_CAMERA_H
#define VKXEL_CAMERA_H

#include "glm/glm.hpp"

#include "transform.h"

namespace Vkxel {

    struct ProjectionInfo {
        float nearClipPlane = 0;
        float farClipPlane = 0;
        float fieldOfViewY = 0;
        float aspect = 0;
    };

    class Camera {
    public:
        Transform transform;
        ProjectionInfo projectionInfo;

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
    };

} // namespace Vkxel

#endif // VKXEL_CAMERA_H
