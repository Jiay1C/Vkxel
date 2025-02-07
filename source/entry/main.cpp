#include <chrono>
#include <format>
#include <thread>

#include "glm/glm.hpp"

#include "engine/application.h"
#include "engine/input.h"
#include "engine/renderer.h"
#include "engine/vtime.h"
#include "world/camera.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/mesh.h"
#include "world/model.h"

using namespace Vkxel;

int main() {

    uint32_t frame_count = 0;
    bool background_mode = false;

    Window window = Window().SetSize(Application::DefaultWindowSize.first, Application::DefaultWindowSize.second)
                            .SetTitle(Application::Name)
                            .AddCallback(WindowEvent::Minimize, [&]() { background_mode = true; })
                            .AddCallback(WindowEvent::Restore, [&]() { background_mode = false; });
    window.Create();
    GUI gui(window);

    Scene scene;
    scene.name = "Main Scene";

    GameObject &camera_game_object = scene.CreateGameObject();
    camera_game_object.name = "Main Camera";
    camera_game_object.transform.position = {0, 0, 1};

    Camera &camera = camera_game_object.AddComponent<Camera>();
    camera.nearClipPlane = Application::DefaultClipPlane.first;
    camera.farClipPlane = Application::DefaultClipPlane.second;
    camera.fieldOfViewY = Application::DefaultFov;
    camera.aspect = window.GetAspect();

    window.AddCallback(WindowEvent::Resize, [&]() { camera.aspect = window.GetAspect(); });

    Controller &camera_controller = camera_game_object.AddComponent<Controller>()
                                            .SetMoveSpeed(Application::DefaultMoveSpeed)
                                            .SetRotateSpeed(Application::DefaultRotateSpeed);

    scene.SetCamera(camera_game_object.name);

    for (int x = -5; x <= 5; ++x) {
        for (int y = -5; y <= 5; ++y) {
            GameObject &bunny = scene.CreateGameObject();
            bunny.name = std::format("Bunny {0},{1}", x, y);
            bunny.transform.position = {x, y, 0};
            bunny.transform.scale = {3, 3, 3};

            Mesh &bunny_mesh = bunny.AddComponent<Mesh>();
            bunny_mesh.index = ModelLibrary::StanfordBunny.index;
            bunny_mesh.vertex = ModelLibrary::StanfordBunny.vertex;

            Drawer &bunny_drawer = bunny.AddComponent<Drawer>();
        }
    }

    scene.Create();

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
    });

    Renderer renderer(window, gui);

    renderer.Init();
    renderer.LoadScene(scene);

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
