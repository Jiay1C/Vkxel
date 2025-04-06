//
// Created by jiayi on 2/9/2025.
//

#include <cstdint>
#include <span>
#include <vector>

#include "glm/glm.hpp"

#include "engine/data_type.h"
#include "engine/engine.h"
#include "gpu_dual_contouring.h"
#include "sdf_surface.h"
#include "util/check.h"
#include "world/gameobject.hpp"
#include "world/mesh.h"

namespace Vkxel {

    void GpuDualContouring::Start() { GenerateMesh(); }

    void GpuDualContouring::GenerateMesh() {
        auto sdf_surface_result = gameObject.GetComponent<SDFSurface>();
        if (!sdf_surface_result) {
            sdf_surface_result = gameObject.AddComponent<SDFSurface>();
        }

        SDFSurface &sdf_surface = sdf_surface_result.value();

        // GPU SDF is currently hard-coded in compute shader
        sdf_surface.surfaceType = SurfaceType::Primitive;
        sdf_surface.primitiveType = PrimitiveType::Sphere;
        _sdf = sdf_surface.GetSDF();

        const glm::ivec3 grid_size = glm::ivec3((maxBound - minBound) * resolution);

        // New Compute Job
        uint32_t grid_elem_num = grid_size.x * grid_size.y * grid_size.z;
        ComputeJob sdfComputeJob = Engine::GetActiveEngine()->GetRenderer().CreateComputeJob();
        sdfComputeJob.Init("compute", {"DualContouringStep0", "DualContouringStep1", "DualContouringStep2"},
                           {sizeof(DualContouringArguments), sizeof(DualContouringResults),
                            sizeof(glm::vec4) * grid_elem_num, sizeof(uint32_t) * grid_elem_num,
                            sizeof(VertexType) * grid_elem_num, sizeof(uint32_t) * grid_elem_num * 18});

        DualContouringArguments arguments = {.gridSize = grid_size,
                                             .minBound = minBound,
                                             .maxBound = maxBound,
                                             .normalDelta = normalDelta,
                                             .schmitzIterationCount = schmitzIterationCount,
                                             .schmitzStepSize = schmitzStepSize};
        DualContouringResults results = {};
        sdfComputeJob.WriteBuffer(0, reinterpret_cast<std::byte *>(&arguments));
        sdfComputeJob.WriteBuffer(1, reinterpret_cast<std::byte *>(&results));

        glm::uvec3 group_size = GetGroupSize(grid_size, {4, 4, 4});
        sdfComputeJob.DispatchImmediate(0, group_size);
        sdfComputeJob.DispatchImmediate(1, group_size);
        sdfComputeJob.DispatchImmediate(2, group_size);

        sdfComputeJob.ReadBuffer(1, reinterpret_cast<std::byte *>(&results));

        std::vector<VertexType> vertices(results.vertexCount);
        sdfComputeJob.ReadBuffer(4, reinterpret_cast<std::byte *>(vertices.data()), 0,
                                 sizeof(VertexType) * results.vertexCount);

        std::vector<IndexType> indices(results.indexCount);
        sdfComputeJob.ReadBuffer(5, reinterpret_cast<std::byte *>(indices.data()), 0,
                                 sizeof(IndexType) * results.indexCount);

        // Assign Vertex and Index to Mesh Component
        if (!gameObject.GetComponent<Mesh>()) {
            gameObject.AddComponent<Mesh>();
        }

        Mesh &mesh = gameObject.GetComponent<Mesh>().value();
        mesh.SetMesh({.index = std::move(indices), .vertex = std::move(vertices)});
    }

    size_t GpuDualContouring::GetIndex1D(const glm::ivec3 &size, const glm::ivec3 &index) {
        return index.x * size.y * size.z + index.y * size.z + index.z;
    }

    glm::uvec3 GpuDualContouring::GetGroupSize(const glm::uvec3 &thread, const glm::uvec3 &threadPerGroup) {
        return (thread + threadPerGroup - glm::uvec3{1}) / threadPerGroup;
    }


} // namespace Vkxel
