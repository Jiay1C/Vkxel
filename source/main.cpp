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
            .position = {0, 0, 1}
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
    while (!renderer.Render()) {
        Input::Update();

        auto this_frame_time = std::chrono::high_resolution_clock::now();
        auto delta_seconds = std::chrono::duration<float>(this_frame_time - last_frame_time).count();
        last_frame_time = this_frame_time;

        float camera_speed = 1.0f;
        float camera_move = camera_speed * delta_seconds;

        if (Input::GetKey(KeyCode::KEY_W)) {
            camera.transform.TranslateSelf(glm::vec3{0, 0, 1} * camera_move);
        }

        if (Input::GetKey(KeyCode::KEY_S)) {
            camera.transform.TranslateSelf(glm::vec3{0, 0, -1} * camera_move);
        }

        if (Input::GetKey(KeyCode::KEY_D)) {
            camera.transform.TranslateSelf(glm::vec3{1, 0, 0} * camera_move);
        }

        if (Input::GetKey(KeyCode::KEY_A)) {
            camera.transform.TranslateSelf(glm::vec3{-1, 0, 0} * camera_move);
        }

        if (Input::GetKey(KeyCode::KEY_Q)) {
            camera.transform.TranslateSelf(glm::vec3{0, 1, 0} * camera_move);
        }

        if (Input::GetKey(KeyCode::KEY_E)) {
            camera.transform.TranslateSelf(glm::vec3{0, -1, 0} * camera_move);
        }
    }
    renderer.ReleaseResource();
    renderer.Destroy();

    return 0;
}
