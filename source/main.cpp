#include "renderer.h"

int main() {
    Vkxel::Renderer renderer;

    renderer.Init();
    renderer.Allocate();
    while (!renderer.Render()) {}
    renderer.Release();
    renderer.Destroy();

    return 0;
}
