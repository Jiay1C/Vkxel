//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_H
#define VKXEL_MODEL_H

#include <vector>

#include "buffer.h"
#include "transform.h"

namespace Vkxel {
    struct Model {
        Transform transform;
        std::vector<uint32_t> index;
        std::vector<VertexInput> vertex;
    };

    class ModelLibrary {
    public:
        ModelLibrary() = delete;
        ~ModelLibrary() = delete;

        static const Model Triangle;

        static const Model StanfordBunny;
    };

} // namespace Vkxel

#endif // VKXEL_MODEL_H
