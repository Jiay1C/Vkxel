//
// Created by jiayi on 2/6/2025.
//

#include <optional>
#include <string_view>
#include <utility>

#include "camera.h"
#include "canvas.h"
#include "drawer.h"
#include "engine/timer.h"
#include "gameobject.hpp"
#include "scene.h"

namespace Vkxel {

    std::optional<std::reference_wrapper<GameObject>> Scene::GetGameObject(const std::string_view gameObjectName) {
        for (auto &gameObject: _gameobjects) {
            if (gameObject.name == gameObjectName) {
                return gameObject;
            }
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<GameObject>> Scene::GetGameObject(const IdType gameObjectId) {
        for (auto &gameObject: _gameobjects) {
            if (gameObject.id == gameObjectId) {
                return gameObject;
            }
        }
        return std::nullopt;
    }

    std::ranges::ref_view<std::list<GameObject>> Scene::GetGameObjectsView() {
        return std::ranges::views::all(_gameobjects);
    }

    GameObject &Scene::CreateGameObject() {
        GameObject &game_object = _gameobjects.emplace_back(*this);
        game_object.Init();
        return game_object;
    }

    GameObject &Scene::AddGameObject(GameObject &&gameObject) {
        return _gameobjects.emplace_back(std::move(gameObject));
    }

    void Scene::DestroyGameObject(const GameObject &gameObject) {
        if (auto it = std::ranges::find_if(_gameobjects, [&](const GameObject &go) { return &go == &gameObject; });
            it != _gameobjects.end()) {
            DestroyGameObjectInternal(it);
        }
    }

    void Scene::DestroyGameObject(const IdType gameObjectId) {
        if (auto it = std::ranges::find_if(_gameobjects, [&](const GameObject &go) { return go.id == gameObjectId; });
            it != _gameobjects.end()) {
            DestroyGameObjectInternal(it);
        }
    }


    void Scene::DestroyGameObject(const std::string_view gameObjectName) {
        if (auto it =
                    std::ranges::find_if(_gameobjects, [&](const GameObject &go) { return go.name == gameObjectName; });
            it != _gameobjects.end()) {
            DestroyGameObjectInternal(it);
        }
    }

    void Scene::DestroyGameObjectInternal(const std::list<GameObject>::iterator &it) {
        for (auto &child: it->transform.GetChildren()) {
            DestroyGameObject(child.get().gameObject);
        }

        Timer::ExecuteAfterTicks(1, [this, it]() {
            it->Destroy();
            _gameobjects.erase(it);
        });
    }

    void Scene::SetCamera(Camera &camera) { _mainCamera = camera; }

    std::optional<std::reference_wrapper<Camera>> Scene::GetCamera() const { return _mainCamera; }

    void Scene::Draw(RenderContext &context) const {
        if (auto camera_result = GetCamera()) {
            const Camera &camera = camera_result.value();
            context.scene = {.viewMatrix = camera.GetViewMatrix(),
                             .projectionMatrix = camera.GetProjectionMatrix(),
                             .cameraPosition = glm::vec4(camera.gameObject.transform.position, 1.0)};
            for (auto &game_object: _gameobjects) {
                if (auto drawer_result = game_object.GetComponent<Drawer>()) {
                    const Drawer &drawer = drawer_result.value();
                    drawer.Draw(context);
                }
                if (auto canvas_result = game_object.GetComponent<Canvas>()) {
                    const Canvas &canvas = canvas_result.value();
                    context.uis += [&]() { canvas.OnGUI(); };
                }
            }
        }
    }


    void Scene::Create() {
        for (auto &game_object: _gameobjects) {
            game_object.Create();
        }
    }

    void Scene::Update() {
        for (auto &game_object: _gameobjects) {
            game_object.Update();
        }
    }

    void Scene::Destroy() {
        for (auto &game_object: _gameobjects) {
            game_object.Destroy();
        }
        _gameobjects.clear();
    }


} // namespace Vkxel
