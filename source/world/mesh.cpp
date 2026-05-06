//
// Created by jiayi on 2/16/2025.
//

#include <utility>
#include <variant>

#include "mesh.h"

namespace Vkxel {

    const std::optional<MeshData> &Mesh::GetMesh() const { return _mesh_data; }

    const MeshBounds &Mesh::GetBounds() const { return _mesh_bounds; }

    uint64_t Mesh::GetRevision() const { return _revision; }

    void Mesh::SetMesh(const MeshData &meshData) {
        _mesh_data = meshData;
        ++_revision;
        UpdateBounds();
    }

    void Mesh::SetMesh(MeshData &&meshData) {
        _mesh_data = std::move(meshData);
        ++_revision;
        UpdateBounds();
    }

    void Mesh::SetBounds(const MeshBounds &bounds) {
        _mesh_bounds = bounds;
        ++_revision;
    }

    void Mesh::UpdateBounds() {
        if (!_mesh_data || !autoUpdateBounds) {
            return;
        }

        if (const auto *cpu_mesh = std::get_if<CPUMeshData>(&_mesh_data.value())) {
            if (cpu_mesh->vertex.empty()) {
                return;
            }

            _mesh_bounds = {.min = cpu_mesh->vertex.front().position, .max = cpu_mesh->vertex.front().position};
            for (const auto &vertex: cpu_mesh->vertex) {
                _mesh_bounds.min = glm::min(_mesh_bounds.min, vertex.position);
                _mesh_bounds.max = glm::max(_mesh_bounds.max, vertex.position);
            }
        }
    }


} // namespace Vkxel
