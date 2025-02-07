//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_MESH_H
#define VKXEL_MESH_H

#include <cstdint>
#include <vector>

#include "component.h"
#include "engine/type.h"

namespace Vkxel {

    class Mesh : public Component {
    public:
        std::vector<uint32_t> index;
        std::vector<VertexData> vertex;
    };

} // namespace Vkxel

#endif // VKXEL_MESH_H
