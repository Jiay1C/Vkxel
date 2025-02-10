//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_DATA_TYPE_H
#define VKXEL_DATA_TYPE_H

#include <vector>

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

namespace Vkxel {

    struct ConstantBufferPerMaterial {};

    struct MaterialData {
        ConstantBufferPerMaterial constantBuffer;
    };

    struct VertexData {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
    };

    struct SceneData {
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::vec4 cameraPosition;
    };

    struct ConstantBufferPerObject {
        glm::mat4 transformMatrix;
    };

    using IndexType = uint32_t;
    using VertexType = VertexData;

    struct ObjectData {
        glm::mat4 transformMatrix;
        const std::vector<IndexType> &index;
        const std::vector<VertexType> &vertex;
    };

    struct ConstantBufferPerFrame {
        SceneData scene;
    };

    struct RenderContext {
        std::vector<ObjectData> objects;
        SceneData scene;
    };

} // namespace Vkxel

#endif // VKXEL_DATA_TYPE_H
