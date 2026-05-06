//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_MESH_H
#define VKXEL_MESH_H

#include <cstdint>
#include <optional>

#include "component.h"
#include "engine/data_type.h"

namespace Vkxel {

    class Mesh final : public Component {
    public:
        using Component::Component;

        const std::optional<MeshData> &GetMesh() const;
        const MeshBounds &GetBounds() const;
        uint64_t GetRevision() const;

        void SetMesh(const MeshData &meshData);
        void SetMesh(MeshData &&meshData);
        void SetBounds(const MeshBounds &bounds);

        bool autoUpdateBounds = true;

    private:
        void UpdateBounds();

        std::optional<MeshData> _mesh_data;
        MeshBounds _mesh_bounds = {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}};
        uint64_t _revision = 0;
    };

    REGISTER_TYPE(Mesh)
    REGISTER_BASE(Component)
    REGISTER_DATA(autoUpdateBounds)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_MESH_H
