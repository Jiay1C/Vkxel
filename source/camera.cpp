//
// Created by jiayi on 1/25/2025.
//

#include "camera.h"

#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"

namespace Vkxel {
    glm::mat4 Camera::GetViewMatrix() const {
        return transform.GetWorldToLocalMatrix();
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        glm::mat4 projectionMatrix = glm::perspectiveRH_ZO(projectionInfo.fieldOfViewY, projectionInfo.aspect, projectionInfo.nearClipPlane, projectionInfo.farClipPlane);
        projectionMatrix[1][1] = -projectionMatrix[1][1];
        return projectionMatrix;
    }


} // Vkxel