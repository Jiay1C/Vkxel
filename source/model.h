//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_H
#define VKXEL_MODEL_H

#include <array>

#include "glm/glm.hpp"

namespace Vkxel {

    struct VertexInput {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
    };

    template<size_t IndexCount, size_t VertexCount>
    struct Model {
        std::array<uint32_t, IndexCount> Index;
        std::array<VertexInput, VertexCount> Vertex;
    };

    class ModelLibrary {
    public:
        ModelLibrary()=delete;
        ~ModelLibrary()=delete;

        static const Model<3,3> Triangle;

        static const Model<14904, 2503> StanfordBunny;

    };

} // Vkxel

#endif //VKXEL_MODEL_H
