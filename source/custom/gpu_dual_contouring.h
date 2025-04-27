//
// Created by jiayi on 3/23/2025.
//

#ifndef VKXEL_GPU_DUAL_CONTOURING_H
#define VKXEL_GPU_DUAL_CONTOURING_H

#include <array>
#include <functional>

#include "glm/glm.hpp"

#include "engine/compute.h"
#include "sdf_surface.h"
#include "world/component.h"

namespace Vkxel {

    class GpuDualContouring final : public Component {
    public:
        using Component::Component;

        bool enableUpdate = false;

        glm::vec3 minBound = glm::vec3{-1};
        glm::vec3 maxBound = glm::vec3{1};
        float resolution = 10;

        float normalDelta = 0.001f;
        uint32_t schmitzIterationCount = 20;
        float schmitzStepSize = 0.1f;

        void Start() override;
        void Update() override;

        void GenerateMesh();
        void RequestOBJ();

    private:
        void ExportOBJ(const std::vector<IndexType>& indices, const std::vector<VertexType>& vertices);
        size_t GetIndex1D(const glm::ivec3 &size, const glm::ivec3 &index);
        glm::uvec3 GetGroupSize(const glm::uvec3 &thread, const glm::uvec3 &threadPerGroup);

        struct DualContouringArguments {
            glm::uvec3 gridSize;
            glm::vec3 minBound;
            glm::vec3 maxBound;
            float normalDelta;
            uint32_t schmitzIterationCount;
            float schmitzStepSize;
            float time;
            uint32_t numSDFSurfaceGPU;
        };

        struct DualContouringResults {
            uint32_t vertexCount;
            uint32_t indexCount;
        };

        glm::vec3 _min_bound_cache = {};
        glm::vec3 _max_bound_cache = {};
        float _resolution_cache = 0;

        const glm::uvec3 _thread_per_group = {4, 4, 4};

        std::optional<ComputeJob> _compute = std::nullopt;

        SDFType _sdf;

        bool _request_obj = false;
    };

    REGISTER_TYPE(GpuDualContouring)
    REGISTER_BASE(Component)
    REGISTER_DATA(enableUpdate)
    REGISTER_DATA(minBound)
    REGISTER_DATA(maxBound)
    REGISTER_DATA(resolution)
    REGISTER_DATA(normalDelta)
    REGISTER_DATA(schmitzIterationCount)
    REGISTER_DATA(schmitzStepSize)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_GPU_DUAL_CONTOURING_H
