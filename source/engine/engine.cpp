//
// Created by jiayi on 2/14/2025.
//


#include <thread>

#include "engine.h"
#include "input.h"
#include "reflect/reflect.hpp"
#include "timer.h"
#include "util/application.h"
#include "util/debug.hpp"
#include "vtime.h"

namespace Vkxel {

    Engine::Engine(Scene &scene) : _scene(scene) {
        s_active_engine = this;

        Reflect::Register();

        Debug::Init();
        Debug::LogInfo("{}::Initializing", Application::Name);

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

        _scene.Start();

        Debug::LogInfo("{}::Running", Application::Name);
    }

    Engine::Status Engine::Tick() {
        s_active_engine = this;

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
            _renderer->WaitIdle();
            constexpr float background_mode_sleep_seconds = 1.0f / Application::BackgroundModeMaxFps;
            std::this_thread::sleep_for(std::chrono::duration<float>(background_mode_sleep_seconds));
            return Status::Sleeping;
        }

        return Status::Running;
    }

    Engine::~Engine() {
        s_active_engine = this;

        Debug::LogInfo("{}::Exiting", Application::Name);

        _renderer->WaitIdle();

        Timer::ImmediateExecute();

        _renderer->UnloadScene();
        _scene.Destroy();
        _renderer->Destroy();

        _window->Destroy();

        s_active_engine = nullptr;
    }

    Scene &Engine::GetScene() const { return _scene; }
    GUI &Engine::GetGUI() const { return *_gui; }
    Window &Engine::GetWindow() const { return *_window; }
    Renderer &Engine::GetRenderer() const { return *_renderer; }
    uint32_t Engine::GetFrameCount() const { return _frame_count; }
    bool Engine::IsBackGroundMode() const { return _background_mode; }
    Engine *Engine::GetActiveEngine() { return s_active_engine; }

    Engine *Engine::s_active_engine = nullptr;

} // namespace Vkxel
