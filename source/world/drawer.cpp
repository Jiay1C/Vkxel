//
// Created by jiayi on 2/5/2025.
//

#include "drawer.h"
#include "gameobject.hpp"
#include "mesh.h"

namespace Vkxel {

    void Drawer::Draw(RenderContext &context) const {
        if (const auto mesh_result = gameObject.GetComponent<Mesh>()) {
            const Mesh &mesh = mesh_result.value();
            const MeshData &mesh_data = mesh.GetMesh();
            if (!mesh_data.index.empty()) {
                context.objects.push_back(
                        {.transform = gameObject.transform.GetLocalToWorldMatrix(), .mesh = mesh_data});
            }
        }
    }

} // namespace Vkxel
