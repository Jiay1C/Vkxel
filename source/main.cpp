#include "application.h"
#include "controller.h"
#include "input.h"
#include "renderer.h"
#include "vtime.h"

using namespace Vkxel;

int main() {

    Window window;

    Camera camera = {
        .transform = {
            .position = glm::vec3{0, 0, 1},
            .rotation = glm::vec3{0, 0, 0}
        },
        .projectionInfo = {
            .nearClipPlane = Application::DefaultClipPlane.first,
            .farClipPlane = Application::DefaultClipPlane.second,
            .fieldOfViewY = Application::DefaultFov,
            .aspect = static_cast<float>(Application::DefaultResolution.first) / Application::DefaultResolution.second
        }
    };

    Controller camera_controller = Controller(camera.transform)
        .SetMoveSpeed(Application::DefaultMoveSpeed)
        .SetRotateSpeed(Application::DefaultRotateSpeed);

    Renderer renderer(window, camera);

    renderer.Init();
    renderer.AllocateResource();
    renderer.UploadData();

    while (!window.ShouldClose()) {
        Input::Update();
        Time::Update();
        camera_controller.Update();

        renderer.Render();

        if (Input::GetKey(KeyCode::KEY_ESCAPE)) {
            window.RequestClose();
        }
    }
    renderer.ReleaseResource();
    renderer.Destroy();

    return 0;
}
