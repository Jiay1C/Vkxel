//
// Created by jiayi on 2/9/2025.
//

#ifndef VKXEL_DUAL_CONTOURING_H
#define VKXEL_DUAL_CONTOURING_H

#include <array>
#include <functional>
#include <utility>

#include "glm/glm.hpp"

#include "sdf_surface.h"
#include "world/component.h"

namespace Vkxel {

    class DualContouring final : public Component {
    public:
        using Component::Component;

        glm::vec3 minBound = glm::vec3{-1};
        glm::vec3 maxBound = glm::vec3{1};
        float resolution = 10;

        float normalDelta = 0.001f;
        uint32_t schmitzIterationCount = 20;
        float schmitzStepSize = 0.1f;

        void Create() override;

        void GenerateMesh();

    private:
        glm::vec3 CalculateNormal(const glm::vec3 &position) const;
        glm::vec3 Grid2World(const glm::vec3 &index) const;

        SDFType _sdf;

        static constexpr std::array<glm::ivec3, 8> _voxel_point{
                {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}, {0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}}};

        static constexpr std::array<glm::ivec2, 12> _voxel_edge{
                {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}}};

        //    (point offset, voxel offset)
        static constexpr std::array<std::pair<glm::ivec3, std::array<glm::ivec3, 4>>, 3> _point_offset{
                {{glm::ivec3{1, 0, 0}, std::array<glm::ivec3, 4>{{{0, -1, -1}, {0, 0, -1}, {0, -1, 0}, {0, 0, 0}}}},
                 {glm::ivec3{0, 1, 0}, std::array<glm::ivec3, 4>{{{-1, 0, -1}, {-1, 0, 0}, {0, 0, -1}, {0, 0, 0}}}},
                 {glm::ivec3{0, 0, 1}, std::array<glm::ivec3, 4>{{{-1, -1, 0}, {0, -1, 0}, {-1, 0, 0}, {0, 0, 0}}}}}};

        static constexpr std::array<uint32_t, 6> _triangle_index_front = {0, 2, 1, 1, 2, 3};
        static constexpr std::array<uint32_t, 6> _triangle_index_back = {0, 1, 2, 1, 3, 2};

        REGISTER_BEGIN(DualContouring)
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

#endif // VKXEL_DUAL_CONTOURING_H
