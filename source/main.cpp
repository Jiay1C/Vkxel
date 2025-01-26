#include "application.h"
#include "renderer.h"

using namespace Vkxel;

int main() {
    Renderer renderer;

    renderer.Init();
    renderer.AllocateResource();
    renderer.UploadData();

    Camera& camera = renderer.GetCamera();
    camera.projectionInfo = {
        .nearClipPlane = Application::DefaultClipPlane.first,
        .farClipPlane = Application::DefaultClipPlane.second,
        .fieldOfViewY = Application::DefaultFov,
        .aspect = static_cast<float>(Application::DefaultResolution.first) / Application::DefaultResolution.second
    };
    camera.transform.SetPosition({0, 0, 1});

    while (!renderer.Render()) {}
    renderer.ReleaseResource();
    renderer.Destroy();

    return 0;
}
