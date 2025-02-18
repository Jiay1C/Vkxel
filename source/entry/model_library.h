//
// Created by jiayi on 1/21/2025.
//

#ifndef VKXEL_MODEL_LIBRARY_H
#define VKXEL_MODEL_LIBRARY_H

#include <vector>

#include "custom/sdf_surface.h"
#include "engine/data_type.h"

namespace Vkxel {

    class ModelLibrary {
    public:
        ModelLibrary() = delete;
        ~ModelLibrary() = delete;

        static const SDFType StanfordBunnySDF;

        static const MeshData TriangleMesh;
        static const MeshData StanfordBunnyMesh;
    };

} // namespace Vkxel

#endif // VKXEL_MODEL_LIBRARY_H
