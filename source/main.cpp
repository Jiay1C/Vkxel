#include "window.h"

int main() {
    Vkxel::Window window;
    window.SetResolution(800, 600);
    window.SetTitle("Vkxel");
    window.Create();

    while (!window.Update()) {
    }

    window.Destroy();
}
