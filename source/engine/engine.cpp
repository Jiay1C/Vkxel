//
// Created by jiayi on 2/14/2025.
//


#include <thread>

#include "engine.h"
#include "input.h"
#include "reflect/reflect.hpp"
#include "timer.h"
#include "util/application.h"
#include "vtime.h"

namespace Vkxel {

    Engine::Engine(Scene &scene) : _scene(scene) {
        Reflect::Register();

        _scene.Init();
        _scene.Create();

        _window = std::make_unique<Window>();
        _window->SetSize(Application::DefaultWindowWidth, Application::DefaultWindowHeight)
                .SetTitle(Application::Name)
                .AddCallback(WindowEvent::Minimize, [&]() { _background_mode = true; })
                .AddCallback(WindowEvent::Restore, [&]() { _background_mode = false; })
                .AddCallback(WindowEvent::Resize, [&]() {
                    if (auto camera_result = _scene.GetCamera()) {
                        Camera &camera = camera_result.value();
                        camera.aspect = _window->GetAspect();
                    } // Ugly code to update camera aspect
                });
        _window->Create();

        _gui = std::make_unique<GUI>(*_window);

        _renderer = std::make_unique<Renderer>(*_window, *_gui);
        _renderer->Init();
        _renderer->LoadScene(scene);
    }

    Engine::Status Engine::Tick() {
        Time::Update();
        Timer::Update();
        Input::Update();

        _window->Update();
        _gui->Update();
        _scene.Update();

        _renderer->Render();

        if (Input::GetKey(KeyCode::KEY_ESCAPE)) {
            _window->RequestClose();
        }

        ++_frame_count;

        if (_window->ShouldClose()) {
            return Status::Exiting;
        }

        if (_background_mode) {
            constexpr float background_mode_sleep_seconds = 1.0f / Application::BackgroundModeMaxFps;
            std::this_thread::sleep_for(std::chrono::duration<float>(background_mode_sleep_seconds));
            return Status::Sleeping;
        }

        return Status::Running;
    }

    Engine::~Engine() {
        _renderer->UnloadScene();
        _renderer->Destroy();
        _scene.Destroy();

        _window->Destroy();
    }

    Scene &Engine::GetScene() const { return _scene; }
    GUI &Engine::GetGUI() const { return *_gui; }
    Window &Engine::GetWindow() const { return *_window; }
    Renderer &Engine::GetRenderer() const { return *_renderer; }
    uint32_t Engine::GetFrameCount() const { return _frame_count; }
    bool Engine::IsBackGroundMode() const { return _background_mode; }


} // namespace Vkxel
