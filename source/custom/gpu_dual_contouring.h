//
// Created by jiayi on 3/23/2025.
//

#ifndef VKXEL_GPU_DUAL_CONTOURING_H
#define VKXEL_GPU_DUAL_CONTOURING_H

#include <array>
#include <functional>
#include <utility>

#include "glm/glm.hpp"

#include "engine/compute.h"
#include "sdf_surface.h"
#include "world/component.h"

namespace Vkxel {

    class GpuDualContouring final : public Component {
    public:
        using Component::Component;

        glm::vec3 minBound = {-1.2, -1.2, -1.2};
        glm::vec3 maxBound = {1.2, 1.2, 1.2};
        float resolution = 10;

        float normalDelta = 0.001f;
        uint32_t schmitzIterationCount = 4;
        float schmitzStepSize = 0.1f;

        void Start() override;

        void GenerateMesh();

    private:
        size_t GetIndex1D(const glm::ivec3 &size, const glm::ivec3 &index);
        glm::uvec3 GetGroupSize(const glm::uvec3 &thread, const glm::uvec3 &threadPerGroup);

        struct DualContouringArguments {
            glm::uvec3 gridSize;
            glm::vec3 minBound;
            glm::vec3 maxBound;
            float normalDelta;
            uint32_t schmitzIterationCount;
            float schmitzStepSize;
        };

        struct DualContouringResults {
            uint32_t vertexCount;
            uint32_t indexCount;
        };

        glm::vec3 _min_bound_cache = {};
        glm::vec3 _max_bound_cache = {};
        float _resolution_cache = 0;

        std::optional<ComputeJob> _compute = std::nullopt;

        SDFType _sdf;

        REGISTER_BEGIN(GpuDualContouring)
        REGISTER_BASE(Component)
        REGISTER_DATA(minBound)
        REGISTER_DATA(maxBound)
        REGISTER_DATA(resolution)
        REGISTER_DATA(normalDelta)
        REGISTER_DATA(schmitzIterationCount)
        REGISTER_DATA(schmitzStepSize)
        REGISTER_END()
    };

} // namespace Vkxel

#endif // VKXEL_GPU_DUAL_CONTOURING_H
