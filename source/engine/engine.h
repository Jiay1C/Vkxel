//
// Created by jiayi on 2/14/2025.
//

#ifndef VKXEL_ENGINE_H
#define VKXEL_ENGINE_H

#include <memory>

#include "gui.h"
#include "renderer.h"
#include "window.h"

namespace Vkxel {

    class Engine {
    public:
        explicit Engine(Scene &scene);

        void Run();

        ~Engine();

        Scene &GetScene() const;
        GUI &GetGUI() const;
        Window &GetWindow() const;
        Renderer &GetRenderer() const;
        uint32_t GetFrameCount() const;
        bool IsBackGroundMode() const;

    private:
        // TODO: Support Multiple Scenes
        Scene &_scene;
        std::unique_ptr<GUI> _gui;
        std::unique_ptr<Window> _window;
        std::unique_ptr<Renderer> _renderer;

        uint32_t _frame_count = 0;
        bool _background_mode = false;
    };

} // namespace Vkxel

#endif // VKXEL_ENGINE_H
