#include "editor/editor.h"
#include "engine/engine.h"
#include "scene_library.h"

using namespace Vkxel;

int main() {

    // Set Up Scene
    Scene scene = SceneLibrary::TestScene();

    // Create Engine
    EditorEngine engine(scene);

    // Main Loop
    while (engine.Tick() != Engine::Status::Exiting) {
    }

    return 0;
}
