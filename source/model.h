//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_H
#define VKXEL_MODEL_H

#include <array>

#include "glm/glm.hpp"

namespace Vkxel {

    template<size_t IndexCount, size_t VertexCount>
    struct Model {
        std::array<uint32_t, IndexCount> Index;
        std::array<glm::vec3, VertexCount> Vertex;
    };

    class ModelLibrary {
    public:
        ModelLibrary()=delete;
        ~ModelLibrary()=delete;

        static const Model<3,3> Triangle;

    };

} // Vkxel

#endif //VKXEL_MODEL_H
