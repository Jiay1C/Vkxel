//
// Created by jiayi on 2/5/2025.
//

#include "drawer.h"
#include "gameobject.hpp"
#include "mesh.h"

namespace Vkxel {

    void Drawer::Draw(RenderContext &context) {
        if (const auto mesh_result = gameObject.GetComponent<Mesh>()) {
            Mesh &mesh = mesh_result.value();
            if (const auto &mesh_data = mesh.GetMesh()) {
                context.objects.push_back({.objectId = gameObject.id,
                                           .transform = gameObject.transform.GetLocalToWorldMatrix(),
                                           .mesh = mesh_data.value(),
                                           .isDirty = mesh.GetRevision() != _last_mesh_revision});
            }
            _last_mesh_revision = mesh.GetRevision();
        }
    }

} // namespace Vkxel
