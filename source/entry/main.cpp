#include <chrono>
#include <format>

#include "glm/glm.hpp"

#include "custom/dual_contouring.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "model_library.h"
#include "world/camera.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/mesh.h"
#include "world/mover.h"

using namespace Vkxel;

int main() {

    // Set Up Scene
    Scene scene;
    scene.name = "Main Scene";

    GameObject &camera_game_object = scene.CreateGameObject();
    camera_game_object.name = "Main Camera";
    camera_game_object.transform.position = {0, 0, 10};

    Camera &camera = camera_game_object.AddComponent<Camera>();
    Controller &camera_controller = camera_game_object.AddComponent<Controller>();

    scene.SetCamera(camera_game_object);

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

    // Create Engine
    Engine engine(scene);
    Window &window = engine.GetWindow();
    GUI &gui = engine.GetGUI();

    window.AddCallback(WindowEvent::Resize, [&]() { camera.aspect = window.GetAspect(); });

    gui.AddStaticItem("Vkxel", [&]() {
        ImGui::Text(std::format("Frame {0} ({1} ms)", Time::GetTicks(), Time::GetRealDeltaSeconds() * 1000).data());
        ImGui::Text(std::format("Size ({0}, {1})", window.GetWidth(), window.GetHeight()).data());
        ImGui::Text(std::format("Resolution ({0}, {1})", window.GetFrameBufferWidth(), window.GetFrameBufferHeight())
                            .data());
        if (ImGui::CollapsingHeader("Camera")) {
            ImGui::DragFloat("Near Clip Plane", &camera.nearClipPlane);
            ImGui::DragFloat("Far Clip Plane", &camera.farClipPlane);

            auto fov = glm::degrees(camera.fieldOfViewY);
            ImGui::DragFloat("Field of View", &fov);
            camera.fieldOfViewY = glm::radians(fov);
        }
        if (ImGui::CollapsingHeader("Controller")) {
            ImGui::DragFloat("Move Speed", &camera_controller.moveSpeed, 0.02f, 0.0f, 10.0f);
            ImGui::DragFloat("Rotate Speed", &camera_controller.rotateSpeed, 0.02f, 0.0f, 10.0f);
            ImGui::DragFloat("Accelerate Ratio", &camera_controller.accelerateRatio, 0.02f, 0.0f, 10.0f);
        }
        if (ImGui::CollapsingHeader("Dual Contouring")) {
            ImGui::DragFloat3("Min Bound", reinterpret_cast<float *>(&dual_contouring.minBound));
            ImGui::DragFloat3("Max Bound", reinterpret_cast<float *>(&dual_contouring.maxBound));
            ImGui::DragFloat("Resolution", &dual_contouring.resolution);
            if (ImGui::Button("Generate Mesh")) {
                dual_contouring.GenerateMesh();
            }
        }
    });

    gui.AddStaticItem("Scene", [&]() {
        for (auto &gameobject: scene.GetGameObjectList()) {
            ImGui::PushID(gameobject.id);
            if (ImGui::TreeNode(std::format("{0} ({1})", gameobject.name, gameobject.id).data())) {
                // Transform
                if (Transform &transform = gameobject.transform;
                    ImGui::TreeNode(std::format("{0} ({1})", transform.name, transform.id).data())) {
                    auto &position = transform.position;
                    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&position));

                    auto rotation = glm::degrees(glm::eulerAngles(transform.rotation));
                    ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&rotation));
                    transform.rotation = glm::radians(rotation);

                    auto &scale = transform.scale;
                    ImGui::DragFloat3("Scale", reinterpret_cast<float *>(&scale));

                    ImGui::TreePop();
                }

                // Other Components
                for (const auto &component: gameobject.GetComponentList()) {
                    ImGui::PushID(component->id);
                    if (ImGui::TreeNode(std::format("{0} ({1})", component->name, component->id).data())) {
                        // TODO: Fix Crash When Remove Component
                        // if (ImGui::Button("Remove")) {
                        //     gameobject.RemoveComponent(*component);
                        // }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                if (ImGui::Button("Destroy")) {
                    scene.DestroyGameObject(gameobject);
                }

                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    });

    // Main Loop
    engine.Run();

    return 0;
}
