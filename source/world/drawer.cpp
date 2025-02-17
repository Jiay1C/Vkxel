//
// Created by jiayi on 2/5/2025.
//

#include "drawer.h"
#include "gameobject.hpp"
#include "mesh.h"

namespace Vkxel {

    void Drawer::Draw(RenderContext &context) const {
        if (const auto mesh_result = gameObject.GetComponent<Mesh>()) {
            if (Mesh &mesh = mesh_result.value(); !mesh.Empty()) {
                const MeshData &mesh_data = mesh.GetMesh();
                context.objects.push_back({.objectId = gameObject.id,
                                           .transform = gameObject.transform.GetLocalToWorldMatrix(),
                                           .mesh = mesh_data,
                                           .isDirty = mesh.GetDirtyFlag()});
                mesh.ClearDirtyFlag();
            }
        }
    }

} // namespace Vkxel
