//
// Created by jiayi on 1/25/2025.
//

#ifndef VKXEL_DATA_TYPE_H
#define VKXEL_DATA_TYPE_H

#include <variant>
#include <vector>

#include "glm/glm.hpp"
#include "util/delegate.hpp"
#include "vkutil/buffer.h"
#include "vulkan/vulkan.h"

namespace Vkxel {

    using IdType = uint64_t;

    using TickType = uint64_t;

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

    struct FrameData {
        uint32_t tick;
        float time;
    };

    struct ConstantBufferPerObject {
        glm::mat4 transformMatrix;
    };

    using IndexType = uint32_t;
    using VertexType = VertexData;

    struct CPUMeshData {
        std::vector<IndexType> index;
        std::vector<VertexType> vertex;
    };

    struct GPUMeshData {
        uint32_t indexCount = 0;
        uint32_t vertexCount = 0;
        VkUtil::Buffer index;
        VkUtil::Buffer vertex;
    };

    using MeshData = std::variant<CPUMeshData, GPUMeshData>;

    struct ObjectData {
        IdType objectId;
        glm::mat4 transform;
        const MeshData &mesh;
        bool isDirty;
    };

    struct ConstantBufferPerFrame {
        FrameData frame;
        SceneData scene;
    };

    struct RenderContext {
        std::vector<ObjectData> objects;
        SceneData scene;
        Delegate<> uis;
    };

} // namespace Vkxel

#endif // VKXEL_DATA_TYPE_H
