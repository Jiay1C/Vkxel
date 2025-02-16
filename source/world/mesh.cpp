//
// Created by jiayi on 2/16/2025.
//

#include "mesh.h"

namespace Vkxel {

    const MeshData &Mesh::GetMesh() const { return _mesh_data; }

    void Mesh::SetMesh(const MeshData &meshData) {
        _mesh_data = meshData;
        _is_dirty = true;
    }

    void Mesh::SetMesh(MeshData &&meshData) {
        _mesh_data = std::move(meshData);
        _is_dirty = true;
    }


    bool Mesh::GetDirtyFlag() const { return _is_dirty; }

    void Mesh::ClearDirtyFlag() { _is_dirty = false; }


} // namespace Vkxel
