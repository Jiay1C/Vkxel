//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_H
#define VKXEL_MODEL_H

#include <vector>

#include "engine/data_type.h"
#include "world/mesh.h"
#include "world/transform.h"

namespace Vkxel {

    class ModelLibrary {
    public:
        ModelLibrary() = delete;
        ~ModelLibrary() = delete;

        static const MeshData Triangle;

        static const MeshData StanfordBunny;
    };

} // namespace Vkxel

#endif // VKXEL_MODEL_H
