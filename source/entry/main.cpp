#include <chrono>
#include <format>

#include "glm/glm.hpp"

#include "custom/dual_contouring.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "model.h"
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

    // Create SDF Object
    GameObject &sdf_object = scene.CreateGameObject();
    sdf_object.name = "SDF Sphere";
    sdf_object.transform.position = {0, 0, 7};
    sdf_object.transform.rotation = glm::radians(glm::vec3{-90, 90, 0});
    Mesh &sdf_mesh = sdf_object.AddComponent<Mesh>();
    sdf_object.AddComponent<Drawer>();

    Mover &sdf_mover = sdf_object.AddComponent<Mover>();
    sdf_mover.angularVelocity = glm::radians(glm::vec3{0, 0, -20});

    DualContouring &dual_contouring = sdf_object.AddComponent<DualContouring>();
    dual_contouring.minBound = {-1, -1, -1};
    dual_contouring.maxBound = {1, 1, 1};
    dual_contouring.resolution = 20;
    dual_contouring.sdf = SDFLibrary::StanfordBunny;

    GameObject &bunny_root = scene.CreateGameObject();
    bunny_root.name = "Bunny Root";

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
                bunny_mesh.SetMesh(ModelLibrary::StanfordBunny);

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
        if (ImGui::CollapsingHeader("Scene")) {
            for (auto &gameobject: scene.GetGameObjectList()) {
                ImGui::PushID(gameobject.id);
                if (ImGui::TreeNode(std::format("{0} ({1})", gameobject.name, gameobject.id).data())) {
                    // Transform
                    if (ImGui::TreeNode(std::format("Transform ({0})", gameobject.transform.id).data())) {
                        auto &position = gameobject.transform.position;
                        ImGui::DragFloat3("Position", reinterpret_cast<float *>(&position));

                        auto rotation = glm::degrees(glm::eulerAngles(gameobject.transform.rotation));
                        ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&rotation));
                        gameobject.transform.rotation = glm::radians(rotation);

                        auto &scale = gameobject.transform.scale;
                        ImGui::DragFloat3("Scale", reinterpret_cast<float *>(&scale));

                        ImGui::TreePop();
                    }

                    // Other Components
                    for (const auto &component: gameobject.GetComponentList()) {
                        ImGui::PushID(component->id);
                        if (ImGui::TreeNode(std::format("{0} ({1})", component->name, component->id).data())) {
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }

                    // Debug Features
                    // if (ImGui::TreeNode("Debug")) {
                    //     if (auto mesh_result = gameobject.GetComponent<Mesh>()) {
                    //         if (ImGui::SmallButton("Apply SDF Mesh")) {
                    //             Mesh &mesh = mesh_result.value();
                    //             mesh.SetMesh(sdf_mesh.GetMesh());
                    //             gameobject.transform.scale = {0.3f, 0.3f, 0.3f};
                    //         }
                    //     }
                    //
                    //     // Destroy Button
                    //     // TODO: Support Destroy
                    //     // if (ImGui::SmallButton("Destroy")) {
                    //     //     scene.DestroyGameObject(gameobject);
                    //     // }
                    //
                    //     ImGui::TreePop();
                    // }

                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
    });

    // Main Loop
    engine.Run();

    return 0;
}
