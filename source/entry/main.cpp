#include <chrono>
#include <format>
#include <thread>

#include "glm/glm.hpp"

#include "custom/dual_contouring.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/vtime.h"
#include "model.h"
#include "util/application.h"
#include "world/camera.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/mesh.h"

using namespace Vkxel;

int main() {

    // Variables

    uint32_t frame_count = 0;
    bool background_mode = false;

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

    scene.Create();


    // Create Engine

    Window window = Window().SetSize(Application::DefaultWindowWidth, Application::DefaultWindowHeight)
                            .SetTitle(Application::Name)
                            .AddCallback(WindowEvent::Minimize, [&]() { background_mode = true; })
                            .AddCallback(WindowEvent::Restore, [&]() { background_mode = false; })
                            .AddCallback(WindowEvent::Resize, [&]() { camera.aspect = window.GetAspect(); });
    window.Create();
    GUI gui(window);

    gui.AddStaticItem("Vkxel", [&]() {
        ImGui::Text(std::format("Frame {0} ({1} ms)", frame_count, Time::RealDeltaSeconds() * 1000).data());
        ImGui::Text(std::format("Size ({0}, {1})", window.GetWidth(), window.GetHeight()).data());
        ImGui::Text(std::format("Resolution ({0}, {1})", window.GetFrameBufferWidth(), window.GetFrameBufferHeight())
                            .data());
        if (ImGui::CollapsingHeader("Camera")) {
            auto position = camera_game_object.transform.position;
            auto rotation = glm::degrees(glm::eulerAngles(camera_game_object.transform.rotation));
            ImGui::InputFloat3("Position", reinterpret_cast<float *>(&position));
            ImGui::InputFloat3("Rotation", reinterpret_cast<float *>(&rotation));
        }
        if (ImGui::CollapsingHeader("Controller")) {
            ImGui::DragFloat("Move Speed", &camera_controller.moveSpeed, 0.02f, 0.0f, 10.0f);
            ImGui::DragFloat("Rotate Speed", &camera_controller.rotateSpeed, 0.02f, 0.0f, 10.0f);
            ImGui::DragFloat("Accelerate Ratio", &camera_controller.accelerateRatio, 0.02f, 0.0f, 10.0f);
        }
    });

    Renderer renderer(window, gui);

    renderer.Init();
    renderer.LoadScene(scene);


    // Main Loop

    while (!window.ShouldClose()) {
        Input::Update();
        Time::Update();
        window.Update();
        gui.Update();
        scene.Update();

        renderer.Render();

        if (Input::GetKey(KeyCode::KEY_ESCAPE)) {
            window.RequestClose();
        }

        ++frame_count;

        if (background_mode) {
            constexpr float background_mode_sleep_seconds = 1.0f / Application::BackgroundModeMaxFps;
            std::this_thread::sleep_for(std::chrono::duration<float>(background_mode_sleep_seconds));
        }
    }

    renderer.UnloadScene();
    renderer.Destroy();
    scene.Destroy();

    window.Destroy();

    return 0;
}
