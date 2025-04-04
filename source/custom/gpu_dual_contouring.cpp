//
// Created by jiayi on 2/9/2025.
//

#include <cstdint>
#include <ranges>
#include <vector>

#include "glm/glm.hpp"

#include "engine/data_type.h"
#include "engine/engine.h"
#include "gpu_dual_contouring.h"
#include "sdf_surface.h"
#include "util/check.h"
#include "world/gameobject.hpp"
#include "world/mesh.h"
#include "world/scene.h"


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
        sdf = sdf_surface.GetSDF();

        const glm::ivec3 grid_size = glm::ivec3((maxBound - minBound) * resolution);

        // New Compute Job
        ComputeJob sdfComputeJob = Engine::GetActiveEngine()->GetRenderer().CreateComputeJob();
        sdfComputeJob.Init("dual_contouring", "computeMain",
                           {sizeof(SDFComputeArgs), sizeof(float) * grid_size.x * grid_size.y * grid_size.z});

        SDFComputeArgs compute_data = {.size = grid_size, .min_bound = minBound, .max_bound = maxBound};
        sdfComputeJob.WriteBuffer(0, reinterpret_cast<std::byte *>(&compute_data));
        sdfComputeJob.DispatchImmediate(grid_size.x >> 2, grid_size.y >> 2, grid_size.z >> 2);
        std::vector<std::byte> compute_output = sdfComputeJob.ReadBuffer(1);

        std::vector<std::vector<std::vector<float>>> grid(
                grid_size.x, std::vector<std::vector<float>>(grid_size.y, std::vector<float>(grid_size.z)));

        for (int x = 0; x < grid_size.x; ++x) {
            for (int y = 0; y < grid_size.y; ++y) {
                for (int z = 0; z < grid_size.z; ++z) {
                    // glm::vec3 position = Grid2World({x, y, z});
                    // grid[x][y][z] = sdf(position);

                    grid[x][y][z] = *reinterpret_cast<float *>(
                            &compute_output[(x * grid_size.y * grid_size.z + y * grid_size.z + z) * sizeof(float)]);
                }
            }
        }

        std::vector<VertexType> vertices;
        std::vector<std::vector<std::vector<IndexType>>> grid_vertex_index(
                grid_size.x - 1,
                std::vector<std::vector<IndexType>>(grid_size.y - 1, std::vector<IndexType>(grid_size.z - 1)));

        // intersect point (grid local position, world normal) on each voxel edge
        std::vector<std::pair<glm::vec3, glm::vec3>> intersections;
        intersections.reserve(_voxel_edge.size());
        for (int x = 0; x < grid_size.x - 1; ++x) {
            for (int y = 0; y < grid_size.y - 1; ++y) {
                for (int z = 0; z < grid_size.z - 1; ++z) {
                    intersections.clear();
                    glm::ivec3 grid_index = {x, y, z};

                    for (const auto &edge: _voxel_edge) {
                        glm::ivec3 p0_local = _voxel_point[edge.x];
                        glm::ivec3 p1_local = _voxel_point[edge.y];

                        glm::ivec3 p0 = grid_index + p0_local;
                        glm::ivec3 p1 = grid_index + p1_local;

                        float p0_value = grid[p0.x][p0.y][p0.z];
                        float p1_value = grid[p1.x][p1.y][p1.z];

                        if ((p0_value <= 0 && p1_value >= 0) || (p0_value >= 0 && p1_value <= 0)) {
                            float interpolate_factor = std::abs(p0_value) / (std::abs(p0_value) + std::abs(p1_value));
                            glm::vec3 p_local = glm::mix(glm::vec3(p0_local), glm::vec3(p1_local), interpolate_factor);
                            glm::vec3 p = glm::vec3(grid_index) + p_local;
                            intersections.emplace_back(p_local, CalculateNormal(Grid2World(p)));
                        }
                    }

                    if (!intersections.empty()) {
                        glm::vec3 center = {};
                        for (const auto &position: intersections | std::views::keys) {
                            center += position;
                        }
                        center /= intersections.size();

                        for (uint32_t count = 0; count < schmitzIterationCount; ++count) {
                            std::array<glm::vec3, 8> force = {};

                            for (const auto &[position, normal]: intersections) {
                                for (uint32_t index = 0; index < 8; ++index) {
                                    float distance = glm::dot(normal, glm::vec3(_voxel_point[index]) - position);
                                    glm::vec3 corner2plane = -distance * normal;
                                    force[index] += corner2plane;
                                }
                            }

                            glm::vec3 force00 = glm::mix(force[0], force[1], center.x);
                            glm::vec3 force01 = glm::mix(force[3], force[2], center.x);
                            glm::vec3 force02 = glm::mix(force[4], force[5], center.x);
                            glm::vec3 force03 = glm::mix(force[7], force[6], center.x);

                            glm::vec3 force10 = glm::mix(force00, force02, center.y);
                            glm::vec3 force11 = glm::mix(force01, force03, center.y);

                            glm::vec3 force20 = glm::mix(force10, force11, center.z);

                            center += force20 * schmitzStepSize;
                        }

                        grid_vertex_index[x][y][z] = static_cast<IndexType>(vertices.size());

                        glm::vec3 position = Grid2World(glm::vec3(grid_index) + center);
                        glm::vec3 normal = CalculateNormal(position);
                        glm::vec3 color = {1, 1, 1};

                        vertices.emplace_back(position, normal, color);
                    } else {
                        grid_vertex_index[x][y][z] = static_cast<IndexType>(~0);
                    }
                }
            }
        }

        std::vector<IndexType> indices;

        for (int x = 1; x < grid_size.x - 1; ++x) {
            for (int y = 1; y < grid_size.y - 1; ++y) {
                for (int z = 1; z < grid_size.z - 1; ++z) {
                    for (const auto &[point_offset, cell_offset]: _point_offset) {
                        glm::ivec3 p0 = {x, y, z};
                        glm::ivec3 p1 = p0 + point_offset;

                        float p0_value = grid[p0.x][p0.y][p0.z];
                        float p1_value = grid[p1.x][p1.y][p1.z];

                        if ((p0_value <= 0 && p1_value >= 0) || (p0_value >= 0 && p1_value <= 0)) {
                            std::array<IndexType, 4> vertex_index = {};

                            for (uint32_t index = 0; index < 4; ++index) {
                                glm::ivec3 grid_index = p0 + cell_offset[index];
                                vertex_index[index] = grid_vertex_index[grid_index.x][grid_index.y][grid_index.z];
                                CHECK_NOTNULL_MSG(vertex_index[index] != static_cast<IndexType>(~0),
                                                  "Invalid Vertex Index");
                            }

                            const auto &triangle_index =
                                    (p0_value >= 0 && p1_value <= 0) ? triangle_index_front : triangle_index_back;

                            for (auto index: triangle_index) {
                                indices.emplace_back(vertex_index[index]);
                            }
                        }
                    }
                }
            }
        }

        // Assign Vertex and Index to Mesh Component
        if (!gameObject.GetComponent<Mesh>()) {
            gameObject.AddComponent<Mesh>();
        }

        Mesh &mesh = gameObject.GetComponent<Mesh>().value();
        mesh.SetMesh({.index = std::move(indices), .vertex = std::move(vertices)});
    }

    glm::vec3 GpuDualContouring::CalculateNormal(const glm::vec3 &position) const {
        glm::vec3 x_delta = {normalDelta, 0, 0};
        glm::vec3 y_delta = {0, normalDelta, 0};
        glm::vec3 z_delta = {0, 0, normalDelta};

        return {(sdf(position + x_delta) - sdf(position - x_delta)) * 0.5f / normalDelta,
                (sdf(position + y_delta) - sdf(position - y_delta)) * 0.5f / normalDelta,
                (sdf(position + z_delta) - sdf(position - z_delta)) * 0.5f / normalDelta};
    }

    glm::vec3 GpuDualContouring::Grid2World(const glm::vec3 &index) const { return index / resolution + minBound; }


} // namespace Vkxel
