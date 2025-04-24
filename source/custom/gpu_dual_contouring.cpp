//
// Created by jiayi on 2/9/2025.
//

#include <cstdint>
#include <span>
#include <vector>

#include "glm/glm.hpp"

#include "engine/data_type.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "gpu_dual_contouring.h"
#include "sdf_surface.h"
#include "world/gameobject.hpp"
#include "world/mesh.h"
#include "util/debug.hpp"

namespace Vkxel {

    void GpuDualContouring::Start() { GenerateMesh(); }

    void GpuDualContouring::Update() {
        if (enableUpdate) {
            GenerateMesh();
        }
    }


    void GpuDualContouring::GenerateMesh() {
        auto sdf_surface_result = gameObject.GetComponent<SDFSurface>();
        if (!sdf_surface_result) {
            sdf_surface_result = gameObject.AddComponent<SDFSurface>();
        }

        SDFSurface &sdf_surface = sdf_surface_result.value();

        // GPU SDF is currently hard-coded in compute shader
        sdf_surface.surfaceType = SurfaceType::CSG;
        sdf_surface.primitiveType = PrimitiveType::Sphere;
        sdf_surface.csgType = CSGType::Unionize;
        // _sdf = sdf_surface.GetSDF();

        std::vector<SDFSurfaceGPU> sdf_surface_gpu;
        sdf_surface.BuildSDFTreeGPU(sdf_surface_gpu, sdf_surface.gameObject.transform.GetLocalToWorldMatrix());
        if (sdf_surface_gpu.empty()) {
            Debug::LogError("SDF Surface GPU is empty!");
            return;
        }

        if (!_compute) {
            _compute = Engine::GetActiveEngine()->GetRenderer().CreateComputeJob();
        }

        ComputeJob &compute_job = _compute.value();

        const glm::ivec3 grid_size = glm::ivec3((maxBound - minBound) * resolution);

        if (minBound != _min_bound_cache || maxBound != _max_bound_cache || resolution != _resolution_cache) {
            _min_bound_cache = minBound;
            _max_bound_cache = maxBound;
            _resolution_cache = resolution;

            uint32_t grid_elem_num = grid_size.x * grid_size.y * grid_size.z;

            compute_job.Init("compute", {"DualContouringStep0", "DualContouringStep1", "DualContouringStep2"},
                             {sizeof(DualContouringArguments), sizeof(DualContouringResults), sizeof(SDFSurfaceGPU) * sdf_surface_gpu.size(),
                              sizeof(glm::vec4) * grid_elem_num, sizeof(uint32_t) * grid_elem_num,
                              sizeof(VertexType) * grid_elem_num, sizeof(uint32_t) * grid_elem_num * 18});
        }
        DualContouringArguments arguments = {.gridSize = grid_size,
                                             .minBound = minBound,
                                             .maxBound = maxBound,
                                             .normalDelta = normalDelta,
                                             .schmitzIterationCount = schmitzIterationCount,
                                             .schmitzStepSize = schmitzStepSize,
                                             .time = Time::GetSeconds(),
                                             .numSDFSurfaceGPU = static_cast<uint32_t>(sdf_surface_gpu.size())};
        DualContouringResults results = {};
        compute_job.WriteBuffer(0, reinterpret_cast<std::byte *>(&arguments));
        compute_job.WriteBuffer(1, reinterpret_cast<std::byte *>(&results));
        compute_job.WriteBuffer(2, reinterpret_cast<std::byte *>(sdf_surface_gpu.data()));

        glm::uvec3 group_size = GetGroupSize(grid_size, _thread_per_group);
        compute_job.DispatchImmediate(0, group_size);
        compute_job.DispatchImmediate(1, group_size);
        compute_job.DispatchImmediate(2, group_size);

        compute_job.ReadBuffer(1, reinterpret_cast<std::byte *>(&results));

        if (results.vertexCount == 0 || results.indexCount == 0) {
            Debug::LogError("Vertex Cound == 0 || Index Count == 0");
            return;
        }

        // Assign Vertex and Index to Mesh Component
        if (!gameObject.GetComponent<Mesh>()) {
            gameObject.AddComponent<Mesh>();
        }

        Mesh &mesh = gameObject.GetComponent<Mesh>().value();
        mesh.SetMesh(GPUMeshData{.indexCount = results.indexCount,
                                 .vertexCount = results.vertexCount,
                                 .index = compute_job.GetBuffer(6),
                                 .vertex = compute_job.GetBuffer(5)});
    }

    size_t GpuDualContouring::GetIndex1D(const glm::ivec3 &size, const glm::ivec3 &index) {
        return index.x * size.y * size.z + index.y * size.z + index.z;
    }

    glm::uvec3 GpuDualContouring::GetGroupSize(const glm::uvec3 &thread, const glm::uvec3 &threadPerGroup) {
        return (thread + threadPerGroup - glm::uvec3{1}) / threadPerGroup;
    }


} // namespace Vkxel
