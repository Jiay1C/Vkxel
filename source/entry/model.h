//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_H
#define VKXEL_MODEL_H

#include <vector>

#include "engine/type.h"
#include "world/mesh.h"
#include "world/transform.h"

namespace Vkxel {

    class ModelLibrary {
    public:
        struct Model {
            std::vector<uint32_t> index;
            std::vector<VertexData> vertex;
        };

        ModelLibrary() = delete;
        ~ModelLibrary() = delete;

        static const Model Triangle;

        static const Model StanfordBunny;
    };

} // namespace Vkxel

#endif // VKXEL_MODEL_H
