//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_MESH_H
#define VKXEL_MESH_H

#include "component.h"
#include "engine/data_type.h"

namespace Vkxel {

    class Mesh final : public Component {
    public:
        using Component::Component;

        const std::optional<MeshData> &GetMesh() const;
        void SetMesh(const MeshData &meshData);
        void SetMesh(MeshData &&meshData);

        bool GetDirtyFlag() const;
        void ClearDirtyFlag();

    private:
        std::optional<MeshData> _mesh_data;
        bool _is_dirty = false;
    };

    REGISTER_TYPE(Mesh)
    REGISTER_BASE(Component)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_MESH_H
