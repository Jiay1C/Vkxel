#include <chrono>

#include "application.h"
#include "renderer.h"
#include "input.h"

using namespace Vkxel;

int main() {
    Renderer renderer;
    Camera& camera = renderer.GetCamera();

    // Set Default Camera Config
    camera = {
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

    renderer.Init();
    renderer.AllocateResource();
    renderer.UploadData();

    auto last_frame_time = std::chrono::high_resolution_clock::now();
    glm::vec2 last_mouse_position;

    while (!renderer.Render()) {
        Input::Update();

        auto this_frame_time = std::chrono::high_resolution_clock::now();
        auto delta_seconds = std::chrono::duration<float>(this_frame_time - last_frame_time).count();
        last_frame_time = this_frame_time;

        float camera_translation = Application::DefaultMoveSpeed * delta_seconds;

        if (Input::GetKey(KeyCode::KEY_W)) {
            camera.transform.TranslateSelf(glm::vec3{0, 0, -1} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_S)) {
            camera.transform.TranslateSelf(glm::vec3{0, 0, 1} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_D)) {
            camera.transform.TranslateSelf(glm::vec3{1, 0, 0} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_A)) {
            camera.transform.TranslateSelf(glm::vec3{-1, 0, 0} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_Q)) {
            camera.transform.TranslateSelf(glm::vec3{0, 1, 0} * camera_translation);
        }

        if (Input::GetKey(KeyCode::KEY_E)) {
            camera.transform.TranslateSelf(glm::vec3{0, -1, 0} * camera_translation);
        }



        glm::vec2 mouse_position = Input::GetMousePosition();
        glm::vec2 mouse_position_delta = mouse_position - last_mouse_position;
        last_mouse_position = mouse_position;

        glm::vec2 camera_rotation = Application::DefaultViewSpeed * delta_seconds * mouse_position_delta;

        if (Input::GetKey(KeyCode::MOUSE_BUTTON_RIGHT)) {
            camera.transform.RotateSelf({-camera_rotation.y, -camera_rotation.x, 0});
        }
    }
    renderer.ReleaseResource();
    renderer.Destroy();

    return 0;
}
