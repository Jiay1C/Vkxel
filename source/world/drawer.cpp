//
// Created by jiayi on 2/5/2025.
//

#include "drawer.h"

#include "gameobject.h"
#include "mesh.h"

namespace Vkxel {

    void Drawer::Draw(RenderContext &context) const {
        if (const auto mesh_result = gameObject.GetComponent<Mesh>()) {
            const Mesh &mesh = mesh_result.value();
            context.objects.push_back({.transformMatrix = gameObject.transform.GetLocalToWorldMatrix(),
                                       .index = mesh.index,
                                       .vertex = mesh.vertex});
        }
    }

} // namespace Vkxel
