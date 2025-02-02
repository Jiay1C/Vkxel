#include <chrono>
#include <format>
#include <thread>

#include "glm/glm.hpp"

#include "application.h"
#include "controller.h"
#include "input.h"
#include "renderer.h"
#include "vtime.h"

using namespace Vkxel;

int main() {

    uint32_t frame_count = 0;
    bool background_mode = false;

    Window window = Window().SetSize(Application::DefaultWindowSize.first, Application::DefaultWindowSize.second)
                            .SetTitle(Application::Name)
                            .AddCallback(WindowEvent::Minimize, [&]() { background_mode = true; })
                            .AddCallback(WindowEvent::Restore, [&]() { background_mode = false; });
    window.Create();

    Camera camera = {.transform = {.position = glm::vec3{0, 0, 1}, .rotation = glm::vec3{0, 0, 0}},
                     .projectionInfo = {.nearClipPlane = Application::DefaultClipPlane.first,
                                        .farClipPlane = Application::DefaultClipPlane.second,
                                        .fieldOfViewY = Application::DefaultFov,
                                        .aspect = window.GetAspect()}};

    window.AddCallback(WindowEvent::Resize, [&]() { camera.projectionInfo.aspect = window.GetAspect(); });

    Controller camera_controller = Controller(camera.transform)
                                           .SetMoveSpeed(Application::DefaultMoveSpeed)
                                           .SetRotateSpeed(Application::DefaultRotateSpeed);

    GUI gui(window);
    gui.AddStaticItem("Vkxel", [&]() {
        ImGui::Text(std::format("Frame {0} ({1} ms)", frame_count, Time::RealDeltaSeconds() * 1000).data());
        ImGui::Text(std::format("Size ({0}, {1})", window.GetWidth(), window.GetHeight()).data());
        ImGui::Text(std::format("Resolution ({0}, {1})", window.GetFrameBufferWidth(), window.GetFrameBufferHeight())
                            .data());
        if (ImGui::CollapsingHeader("Camera")) {
            auto position = camera.transform.position;
            auto rotation = glm::degrees(glm::eulerAngles(camera.transform.rotation));
            ImGui::InputFloat3("Position", reinterpret_cast<float *>(&position));
            ImGui::InputFloat3("Rotation", reinterpret_cast<float *>(&rotation));
        }
    });

    Renderer renderer(window, camera, gui);

    renderer.Init();
    renderer.AllocateResource();
    renderer.UploadData();

    while (!window.ShouldClose()) {
        Input::Update();
        Time::Update();
        window.Update();
        camera_controller.Update();
        gui.Update();

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
    renderer.ReleaseResource();
    renderer.Destroy();

    window.Destroy();

    return 0;
}
