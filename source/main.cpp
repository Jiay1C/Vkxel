#include <format>

#include "glm/glm.hpp"

#include "application.h"
#include "controller.h"
#include "input.h"
#include "renderer.h"
#include "vtime.h"

using namespace Vkxel;

int main() {

    uint32_t frame_count = 0;

    Window window = Window().SetResolution(Application::DefaultResolution.first, Application::DefaultResolution.second)
                            .SetTitle(Application::Name);
    window.Create();

    Camera camera = {.transform = {.position = glm::vec3{0, 0, 1}, .rotation = glm::vec3{0, 0, 0}},
                     .projectionInfo = {.nearClipPlane = Application::DefaultClipPlane.first,
                                        .farClipPlane = Application::DefaultClipPlane.second,
                                        .fieldOfViewY = Application::DefaultFov,
                                        .aspect = static_cast<float>(Application::DefaultResolution.first) /
                                                  Application::DefaultResolution.second}};

    Controller camera_controller = Controller(camera.transform)
                                           .SetMoveSpeed(Application::DefaultMoveSpeed)
                                           .SetRotateSpeed(Application::DefaultRotateSpeed);

    GUI gui(window);
    gui.AddStaticItem("Vkxel", [&]() {
        ImGui::Text(std::format("Frame {0} ({1} ms)", frame_count, Time::RealDeltaSeconds() * 1000).data());
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
        camera_controller.Update();
        gui.Update();

        renderer.Render();

        if (Input::GetKey(KeyCode::KEY_ESCAPE)) {
            window.RequestClose();
        }

        ++frame_count;
    }
    renderer.ReleaseResource();
    renderer.Destroy();

    window.Destroy();

    return 0;
}
