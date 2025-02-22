//
// Created by jiayi on 2/21/2025.
//

#ifndef VKXEL_EDITOR_H
#define VKXEL_EDITOR_H

#include "engine/engine.h"

namespace Vkxel {

    class EditorEngine : public Engine {
    public:
        explicit EditorEngine(Scene &scene);

    private:
        void SetupDebugUI() const;
        void SetupSceneUI() const;
    };

} // namespace Vkxel

#endif // VKXEL_EDITOR_H
