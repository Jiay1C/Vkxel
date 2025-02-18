//
// Created by jiayi on 2/6/2025.
//

#include <optional>
#include <string_view>
#include <utility>

#include "camera.h"
#include "drawer.h"
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

    std::list<GameObject> &Scene::GetGameObjectList() { return _gameobjects; }

    GameObject &Scene::CreateGameObject() { return _gameobjects.emplace_back(*this); }

    GameObject &Scene::AddGameObject(GameObject &&gameObject) {
        return _gameobjects.emplace_back(std::move(gameObject));
    }

    void Scene::DestroyGameObject(const GameObject &gameObject) {
        if (auto it = std::ranges::find_if(_gameobjects, [&](const GameObject &go) { return &go == &gameObject; });
            it != _gameobjects.end()) {
            for (auto &child: it->transform.GetChildren()) {
                DestroyGameObject(child.get().gameObject);
            }

            _destroyed_gameobjects.push_back(it);
        }
    }

    void Scene::DestroyGameObject(IdType gameObjectId) {
        if (auto it = std::ranges::find_if(_gameobjects, [&](const GameObject &go) { return go.id == gameObjectId; });
            it != _gameobjects.end()) {
            for (auto &child: it->transform.GetChildren()) {
                DestroyGameObject(child.get().gameObject);
            }

            _destroyed_gameobjects.push_back(it);
        }
    }


    void Scene::DestroyGameObject(const std::string_view gameObjectName) {
        if (auto it =
                    std::ranges::find_if(_gameobjects, [&](const GameObject &go) { return go.name == gameObjectName; });
            it != _gameobjects.end()) {
            for (auto &child: it->transform.GetChildren()) {
                DestroyGameObject(child.get().gameObject);
            }

            _destroyed_gameobjects.push_back(it);
        }
    }

    void Scene::SetCamera(const GameObject &cameraObject) {
        _mainCamera = std::nullopt;
        for (auto &game_object: _gameobjects) {
            if (&game_object == &cameraObject && game_object.GetComponent<Camera>()) {
                _mainCamera = game_object;
                return;
            }
        }
        CHECK_NOTNULL_MSG(false, "Camera Not Found");
    }

    void Scene::SetCamera(const IdType cameraObjectId) {
        _mainCamera = std::nullopt;
        for (auto &game_object: _gameobjects) {
            if (game_object.id == cameraObjectId && game_object.GetComponent<Camera>()) {
                _mainCamera = game_object;
                return;
            }
        }
        CHECK_NOTNULL_MSG(false, "Camera Not Found");
    }


    void Scene::SetCamera(const std::string_view cameraObjectName) {
        _mainCamera = std::nullopt;
        for (auto &game_object: _gameobjects) {
            if (game_object.name == cameraObjectName && game_object.GetComponent<Camera>()) {
                _mainCamera = game_object;
                return;
            }
        }
        CHECK_NOTNULL_MSG(false, "Camera Not Found");
    }

    std::optional<std::reference_wrapper<const GameObject>> Scene::GetCamera() const { return _mainCamera; }

    void Scene::Draw(RenderContext &context) const {
        if (auto camera_result = GetCamera()) {
            const GameObject &camera_game_object = camera_result.value();
            const Camera &camera = camera_game_object.GetComponent<Camera>().value();
            context.scene = {.viewMatrix = camera.GetViewMatrix(),
                             .projectionMatrix = camera.GetProjectionMatrix(),
                             .cameraPosition = glm::vec4(camera.gameObject.transform.position, 1.0)};
            for (auto &game_object: _gameobjects) {
                if (auto drawer_result = game_object.GetComponent<Drawer>()) {
                    const Drawer &drawer = drawer_result.value();
                    drawer.Draw(context);
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
        // Destroy GameObjects
        for (const auto &it: _destroyed_gameobjects) {
            it->Destroy();
            _gameobjects.erase(it);
        }
        _destroyed_gameobjects.clear();

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
