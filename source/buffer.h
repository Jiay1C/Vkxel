//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_BUFFER_H
#define VKXEL_BUFFER_H

#include "glm/glm.hpp"

namespace Vkxel {

    struct ConstantBufferPerFrame {
        glm::mat4 ModelMatrix;
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
    };

    struct VertexInput {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
    };

} // namespace Vkxel

#endif // VKXEL_BUFFER_H
