//
// Created by jiayi on 2/6/2025.
//

#include "scene.h"

#include "camera.h"
#include "drawer.h"

namespace Vkxel {

    GameObject &Scene::CreateGameObject() { return _gameobjects.emplace_back(); }

    GameObject &Scene::AddGameObject(GameObject &&gameObject) {
        return _gameobjects.emplace_back(std::move(gameObject));
    }

    void Scene::DestroyGameObject(const std::string_view gameobjectName) {
        for (auto it = _gameobjects.begin(); it != _gameobjects.end(); ++it) {
            auto &game_object = *it;
            if (game_object.name == gameobjectName) {
                game_object.Destroy();
                _gameobjects.erase(it);
            }
        }
    }

    void Scene::SetCamera(const std::string_view gameobjectName) {
        _mainCamera = std::nullopt;
        for (auto &game_object: _gameobjects) {
            if (game_object.name == gameobjectName && game_object.GetComponent<Camera>()) {
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
                             .cameraPosition = glm::vec4(camera.gameObject->transform.position, 1.0)};
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
