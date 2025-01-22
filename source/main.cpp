#include "renderer.h"

int main() {
    Vkxel::Renderer renderer;

    renderer.Init();
    renderer.Allocate();
    renderer.Upload();
    while (!renderer.Render()) {}
    renderer.Release();
    renderer.Destroy();

    return 0;
}
