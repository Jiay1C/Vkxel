//
// Created by jiayi on 2/22/2025.
//

#ifndef VKXEL_SCENE_LIBRARY_H
#define VKXEL_SCENE_LIBRARY_H

#include "world/scene.h"

namespace Vkxel {

    class SceneLibrary {
    public:
        SceneLibrary() = delete;
        ~SceneLibrary() = delete;

        static Scene TestScene();
    };

} // namespace Vkxel

#endif // VKXEL_SCENE_LIBRARY_H
