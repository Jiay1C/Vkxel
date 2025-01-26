//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_H
#define VKXEL_MODEL_H

#include <array>

#include "buffer.h"
#include "transform.h"

namespace Vkxel {

    template<uint32_t IndexCount, uint32_t VertexCount>
    struct Model {
        // TODO: Fix Alignment Issue
        // Transform transform;

        const uint32_t indexCount = IndexCount;
        const uint32_t vertexCount = VertexCount;

        struct {
            std::array<uint32_t, IndexCount> index;
            std::array<VertexInput, VertexCount> vertex;
        }buffer;
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
