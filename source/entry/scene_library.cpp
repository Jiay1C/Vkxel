//
// Created by jiayi on 2/22/2025.
//

#include <array>
#include <format>

#include "custom/dual_contouring.h"
#include "custom/gpu_dual_contouring.h"
#include "model_library.h"
#include "scene_library.h"
#include "world/camera.h"
#include "world/canvas.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/gameobject.hpp"
#include "world/mesh.h"
#include "world/mover.h"
#include "world/rigidbody.h"
#include "world/scene.h"

namespace Vkxel {

    namespace {

        CPUMeshData CreateBoxMesh(const glm::vec3 &halfExtent, const glm::vec3 &color) {
            CPUMeshData mesh;

            const auto add_face = [&](const glm::vec3 &normal, const glm::vec3 &a, const glm::vec3 &b,
                                      const glm::vec3 &c, const glm::vec3 &d) {
                const IndexType base = static_cast<IndexType>(mesh.vertex.size());
                mesh.vertex.emplace_back(VertexData{a, normal, color});
                mesh.vertex.emplace_back(VertexData{b, normal, color});
                mesh.vertex.emplace_back(VertexData{c, normal, color});
                mesh.vertex.emplace_back(VertexData{d, normal, color});
                mesh.index.insert(mesh.index.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
            };

            const glm::vec3 p000{-halfExtent.x, -halfExtent.y, -halfExtent.z};
            const glm::vec3 p001{-halfExtent.x, -halfExtent.y, halfExtent.z};
            const glm::vec3 p010{-halfExtent.x, halfExtent.y, -halfExtent.z};
            const glm::vec3 p011{-halfExtent.x, halfExtent.y, halfExtent.z};
            const glm::vec3 p100{halfExtent.x, -halfExtent.y, -halfExtent.z};
            const glm::vec3 p101{halfExtent.x, -halfExtent.y, halfExtent.z};
            const glm::vec3 p110{halfExtent.x, halfExtent.y, -halfExtent.z};
            const glm::vec3 p111{halfExtent.x, halfExtent.y, halfExtent.z};

            add_face({0.0f, 0.0f, 1.0f}, p001, p101, p111, p011);
            add_face({0.0f, 0.0f, -1.0f}, p100, p000, p010, p110);
            add_face({1.0f, 0.0f, 0.0f}, p101, p100, p110, p111);
            add_face({-1.0f, 0.0f, 0.0f}, p000, p001, p011, p010);
            add_face({0.0f, 1.0f, 0.0f}, p011, p111, p110, p010);
            add_face({0.0f, -1.0f, 0.0f}, p000, p100, p101, p001);

            return mesh;
        }

    } // namespace

    Scene SceneLibrary::TestScene() {
        Scene scene;
        scene.name = "Test Scene";

        GameObject &camera_object = scene.CreateGameObject();
        camera_object.name = "Main Camera";
        camera_object.transform.position = {0, 0, 10};
        Camera &camera = camera_object.AddComponent<Camera>();
        camera_object.AddComponent<Controller>();

        scene.SetCamera(camera);

        GameObject &root_object = scene.CreateGameObject();
        root_object.name = "Root";

        GameObject &physics_floor = scene.CreateGameObject();
        physics_floor.name = "Physics Floor";
        physics_floor.transform.SetParent(root_object.transform);
        physics_floor.transform.position = {0.0f, -2.5f, 7.0f};
        physics_floor.AddComponent<Mesh>().SetMesh(CreateBoxMesh({5.0f, 0.12f, 3.0f}, {0.34f, 0.39f, 0.42f}));
        physics_floor.AddComponent<Drawer>();

        Rigidbody &floor_body = physics_floor.AddComponent<Rigidbody>();
        floor_body.config.motionType = RigidbodyMotionType::Static;
        floor_body.config.colliderType = RigidbodyColliderType::TriangleMesh;
        floor_body.config.friction = 0.85f;
        floor_body.config.restitution = 0.05f;

        constexpr std::array cube_colors{
                glm::vec3{0.96f, 0.42f, 0.28f}, glm::vec3{0.20f, 0.62f, 0.86f}, glm::vec3{0.35f, 0.78f, 0.45f},
                glm::vec3{0.92f, 0.72f, 0.22f}, glm::vec3{0.70f, 0.48f, 0.92f}, glm::vec3{0.88f, 0.38f, 0.62f},
        };
        for (uint32_t index = 0; index < cube_colors.size(); ++index) {
            GameObject &cube = scene.CreateGameObject();
            cube.name = std::format("Physics Cube {}", index);
            cube.transform.SetParent(root_object.transform);
            cube.transform.position = {-1.75f + static_cast<float>(index) * 0.7f,
                                       1.1f + static_cast<float>(index) * 0.42f,
                                       6.4f + static_cast<float>(index % 2) * 0.5f};
            cube.transform.rotation =
                    glm::radians(glm::vec3{12.0f * static_cast<float>(index), 21.0f * static_cast<float>(index), 8.0f});
            cube.AddComponent<Mesh>().SetMesh(CreateBoxMesh(glm::vec3{0.35f}, cube_colors[index]));
            cube.AddComponent<Drawer>();

            Rigidbody &cube_body = cube.AddComponent<Rigidbody>();
            cube_body.config.motionType = RigidbodyMotionType::Dynamic;
            cube_body.config.colliderType = RigidbodyColliderType::BoundsBox;
            cube_body.config.mass = 0.8f + static_cast<float>(index) * 0.15f;
            cube_body.config.friction = 0.55f;
            cube_body.config.restitution = 0.25f;
            cube_body.config.angularVelocity =
                    glm::radians(glm::vec3{20.0f + 7.0f * static_cast<float>(index), 35.0f, 12.0f});
        }

        for (uint32_t index = 0; index < 3; ++index) {
            GameObject &bunny = scene.CreateGameObject();
            bunny.name = std::format("Physics Bunny {}", index);
            bunny.transform.SetParent(root_object.transform);
            bunny.transform.position = {-1.2f + static_cast<float>(index) * 1.2f,
                                        3.7f + static_cast<float>(index) * 0.55f, 8.55f};
            bunny.transform.rotation = glm::radians(glm::vec3{0.0f, 35.0f * static_cast<float>(index), 0.0f});
            bunny.transform.scale = {4.0f, 4.0f, 4.0f};

            Mesh &bunny_mesh = bunny.AddComponent<Mesh>();
            bunny_mesh.SetMesh(ModelLibrary::StanfordBunnyMesh);
            bunny.AddComponent<Drawer>();

            Rigidbody &bunny_body = bunny.AddComponent<Rigidbody>();
            bunny_body.config.motionType = RigidbodyMotionType::Dynamic;
            bunny_body.config.colliderType = RigidbodyColliderType::TriangleMesh;
            bunny_body.config.mass = 0.7f;
            bunny_body.config.friction = 0.45f;
            bunny_body.config.restitution = 0.3f;
            bunny_body.config.angularVelocity = glm::radians(glm::vec3{5.0f, 40.0f, 18.0f});
        }

        // Create GPU SDF Object
        GameObject &gpu_sdf_object = scene.CreateGameObject();
        gpu_sdf_object.name = "GPU SDF Object";
        gpu_sdf_object.transform.SetParent(root_object.transform);
        gpu_sdf_object.transform.position = {-2, 0, 7};
        gpu_sdf_object.AddComponent<Mesh>();
        gpu_sdf_object.AddComponent<Drawer>();

        GpuDualContouring &gpu_dual_contouring = gpu_sdf_object.AddComponent<GpuDualContouring>();
        gpu_dual_contouring.enableUpdate = true;
        gpu_dual_contouring.minBound = glm::vec3{-2.5f, -1.2f, -1.2f};
        gpu_dual_contouring.maxBound = glm::vec3{2.5f, 1.2f, 1.2f};
        gpu_dual_contouring.resolution = 20;

        gpu_sdf_object.AddComponent<Canvas>().uiItems += [&]() {
            if (ImGui::Button("GPU Generate Mesh")) {
                gpu_dual_contouring.GenerateMesh();
            }
        };

        // Create SDF Object
        GameObject &sdf_object = scene.CreateGameObject();
        sdf_object.name = "SDF Object";
        sdf_object.transform.SetParent(root_object.transform);
        sdf_object.transform.position = {2, 0, 7};
        sdf_object.AddComponent<Mesh>();
        sdf_object.AddComponent<Drawer>();

        Mover &sdf_mover = sdf_object.AddComponent<Mover>();
        sdf_mover.angularVelocity = glm::radians(glm::vec3{0, 20, 0});

        SDFSurface &sdf_surface = sdf_object.AddComponent<SDFSurface>();
        sdf_surface.surfaceType = SurfaceType::CSG;
        sdf_surface.csgType = CSGType::Subtract;
        sdf_surface.csgSmoothFactor = 0.1f;

        DualContouring &dual_contouring = sdf_object.AddComponent<DualContouring>();
        dual_contouring.minBound = glm::vec3{-1.2f};
        dual_contouring.maxBound = glm::vec3{1.2f};
        dual_contouring.resolution = 20;

        sdf_object.AddComponent<Canvas>().uiItems += [&]() {
            if (ImGui::Button("Generate Mesh")) {
                dual_contouring.GenerateMesh();
            }
        };

        // GameObject &sdf_bunny = scene.CreateGameObject();
        // sdf_bunny.name = "SDF Bunny";
        // sdf_bunny.transform.SetParent(sdf_object.transform);
        // sdf_bunny.transform.rotation = glm::radians(glm::vec3{-90, 90, 0});
        // SDFSurface &sdf_bunny_surface = sdf_bunny.AddComponent<SDFSurface>();
        // sdf_bunny_surface.surfaceType = SurfaceType::Custom;
        // sdf_bunny_surface.customSDF = ModelLibrary::StanfordBunnySDF;

        GameObject &sdf_box = scene.CreateGameObject();
        sdf_box.name = "SDF Box";
        sdf_box.transform.SetParent(sdf_object.transform);
        sdf_box.transform.scale = {0.6f, 0.6f, 0.6f};
        SDFSurface &sdf_box_surface = sdf_box.AddComponent<SDFSurface>();
        sdf_box_surface.surfaceType = SurfaceType::Primitive;
        sdf_box_surface.primitiveType = PrimitiveType::Box;

        GameObject &sdf_object_2 = scene.CreateGameObject();
        sdf_object_2.name = "SDF Object 2";
        sdf_object_2.transform.SetParent(sdf_object.transform);
        SDFSurface &sdf_surface_2 = sdf_object_2.AddComponent<SDFSurface>();
        sdf_surface_2.surfaceType = SurfaceType::CSG;
        sdf_surface_2.csgType = CSGType::Unionize;

        GameObject &sdf_sphere = scene.CreateGameObject();
        sdf_sphere.name = "SDF Sphere";
        sdf_sphere.transform.SetParent(sdf_object_2.transform);
        sdf_sphere.transform.position = {0.5f, 0.5f, 0.2f};
        sdf_sphere.transform.scale = {0.2f, 0.2f, 0.2f};
        SDFSurface &sdf_sphere_surface = sdf_sphere.AddComponent<SDFSurface>();
        sdf_sphere_surface.surfaceType = SurfaceType::Primitive;
        sdf_sphere_surface.primitiveType = PrimitiveType::Sphere;

        GameObject &sdf_capsule = scene.CreateGameObject();
        sdf_capsule.name = "SDF Capsule";
        sdf_capsule.transform.SetParent(sdf_object_2.transform);
        sdf_capsule.transform.position = {-0.6f, 0.2f, 0.2f};
        sdf_capsule.transform.scale = {0.3f, 0.4f, 0.3f};
        SDFSurface &sdf_capsule_surface = sdf_capsule.AddComponent<SDFSurface>();
        sdf_capsule_surface.surfaceType = SurfaceType::Primitive;
        sdf_capsule_surface.primitiveType = PrimitiveType::Capsule;

        GameObject &bunny_root = scene.CreateGameObject();
        bunny_root.name = "Bunny Root";
        bunny_root.transform.SetParent(root_object.transform);

        Mover &bunny_mover = bunny_root.AddComponent<Mover>();
        bunny_mover.angularVelocity = glm::radians(glm::vec3{20, 20, 20});

        // Create Bunny Matrix
        int bunny_axis_count = 3;
        for (int x = -bunny_axis_count; x <= bunny_axis_count; ++x) {
            for (int y = -bunny_axis_count; y <= bunny_axis_count; ++y) {
                for (int z = -bunny_axis_count; z <= bunny_axis_count; ++z) {
                    GameObject &bunny = scene.CreateGameObject();
                    bunny.name = std::format("Bunny {0},{1},{2}", x, y, z);
                    bunny.transform.SetParent(bunny_root.transform);
                    bunny.transform.position = {x, y, z};
                    bunny.transform.scale = {3, 3, 3};

                    Mesh &bunny_mesh = bunny.AddComponent<Mesh>();
                    bunny_mesh.SetMesh(ModelLibrary::StanfordBunnyMesh);

                    bunny.AddComponent<Drawer>();
                }
            }
        }
        return scene;
    }


} // namespace Vkxel
