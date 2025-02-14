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

    GameObject &bunny_root = scene.CreateGameObject();
    bunny_root.name = "Bunny Root";

    // Create Bunny
    for (int x = -5; x <= 5; ++x) {
        for (int y = -5; y <= 5; ++y) {
            GameObject &bunny = scene.CreateGameObject();
            bunny.name = std::format("Bunny {0},{1}", x, y);
            bunny.transform.SetParent(bunny_root.transform);
            bunny.transform.position = {x, y, 0};
            bunny.transform.scale = {3, 3, 3};

            Mesh &bunny_mesh = bunny.AddComponent<Mesh>();
            bunny_mesh.index = ModelLibrary::StanfordBunny.index;
            bunny_mesh.vertex = ModelLibrary::StanfordBunny.vertex;

            bunny.AddComponent<Drawer>();
        }
    }

    // Create SDF Object
    GameObject &sdf_object = scene.CreateGameObject();
    sdf_object.name = "SDF Sphere";
    sdf_object.transform.position = {0, 0, 5};
    sdf_object.transform.rotation = glm::radians(glm::vec3{-90, 90, 0});
    sdf_object.AddComponent<Mesh>();
    sdf_object.AddComponent<Drawer>();
    DualContouring &dual_contouring = sdf_object.AddComponent<DualContouring>();
    dual_contouring.minBound = {-1, -1, -1};
    dual_contouring.maxBound = {1, 1, 1};
    dual_contouring.resolution = 20;
    dual_contouring.sdf = SDFLibrary::StanfordBunny;

    // Create Engine
    Engine engine(scene);
    Window &window = engine.GetWindow();
    GUI &gui = engine.GetGUI();

    window.AddCallback(WindowEvent::Resize, [&]() { camera.aspect = window.GetAspect(); });

    gui.AddStaticItem("Vkxel", [&]() {
        ImGui::Text(std::format("Frame {0} ({1} ms)", engine.GetFrameCount(), Time::RealDeltaSeconds() * 1000).data());
        ImGui::Text(std::format("Size ({0}, {1})", window.GetWidth(), window.GetHeight()).data());
        ImGui::Text(std::format("Resolution ({0}, {1})", window.GetFrameBufferWidth(), window.GetFrameBufferHeight())
                            .data());
        if (ImGui::CollapsingHeader("Camera")) {
            auto &position = camera_game_object.transform.position;
            ImGui::DragFloat3("Position", reinterpret_cast<float *>(&position));

            auto rotation = glm::degrees(glm::eulerAngles(camera_game_object.transform.rotation));
            ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&rotation));
            camera_game_object.transform.rotation = glm::radians(rotation);

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
    });

    // Main Loop
    engine.Run();

    return 0;
}
