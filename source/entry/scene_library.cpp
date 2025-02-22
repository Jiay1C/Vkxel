//
// Created by jiayi on 2/22/2025.
//

#include <format>

#include "custom/dual_contouring.h"
#include "model_library.h"
#include "scene_library.h"
#include "world/camera.h"
#include "world/canvas.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/mesh.h"
#include "world/mover.h"

namespace Vkxel {

    Scene SceneLibrary::TestScene() {
        Scene scene;
        scene.name = "Test Scene";

        GameObject &camera = scene.CreateGameObject();
        camera.name = "Main Camera";
        camera.transform.position = {0, 0, 10};
        camera.AddComponent<Camera>();
        camera.AddComponent<Controller>();

        scene.SetCamera(camera);

        GameObject &root_object = scene.CreateGameObject();
        root_object.name = "Root";

        // Create SDF Object
        GameObject &sdf_object = scene.CreateGameObject();
        sdf_object.name = "SDF Object";
        sdf_object.transform.SetParent(root_object.transform);
        sdf_object.transform.position = {0, 0, 7};
        sdf_object.AddComponent<Mesh>();
        sdf_object.AddComponent<Drawer>();

        Mover &sdf_mover = sdf_object.AddComponent<Mover>();
        sdf_mover.angularVelocity = glm::radians(glm::vec3{0, 20, 0});

        SDFSurface &sdf_surface = sdf_object.AddComponent<SDFSurface>();
        sdf_surface.surfaceType = SurfaceType::CSG;
        sdf_surface.csgType = CSGType::Union;

        DualContouring &dual_contouring = sdf_object.AddComponent<DualContouring>();
        dual_contouring.minBound = {-1, -1, -1};
        dual_contouring.maxBound = {1, 1, 1};
        dual_contouring.resolution = 20;

        Canvas &canvas = sdf_object.AddComponent<Canvas>();
        canvas.uiItems += [&]() {
            ImGui::DragFloat3("Min Bound", reinterpret_cast<float *>(&dual_contouring.minBound));
            ImGui::DragFloat3("Max Bound", reinterpret_cast<float *>(&dual_contouring.maxBound));
            ImGui::DragFloat("Resolution", &dual_contouring.resolution);
            if (ImGui::Button("Generate Mesh")) {
                dual_contouring.GenerateMesh();
            }
        };

        GameObject &sdf_bunny = scene.CreateGameObject();
        sdf_bunny.name = "SDF Bunny";
        sdf_bunny.transform.SetParent(sdf_object.transform);
        sdf_bunny.transform.rotation = glm::radians(glm::vec3{-90, 90, 0});
        SDFSurface &sdf_bunny_surface = sdf_bunny.AddComponent<SDFSurface>();
        sdf_bunny_surface.surfaceType = SurfaceType::Custom;
        sdf_bunny_surface.customSDF = ModelLibrary::StanfordBunnySDF;

        GameObject &sdf_sphere = scene.CreateGameObject();
        sdf_sphere.name = "SDF Sphere";
        sdf_sphere.transform.SetParent(sdf_object.transform);
        sdf_sphere.transform.position = {0.5f, 0.5f, 0.2f};
        sdf_sphere.transform.scale = {0.2f, 0.2f, 0.2f};
        SDFSurface &sdf_sphere_surface = sdf_sphere.AddComponent<SDFSurface>();
        sdf_sphere_surface.surfaceType = SurfaceType::Primitive;
        sdf_sphere_surface.primitiveType = PrimitiveType::Sphere;

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
