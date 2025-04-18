//
// Created by jiayi on 2/6/2025.
//

#ifndef VKXEL_SCENE_H
#define VKXEL_SCENE_H

#include <list>
#include <optional>
#include <ranges>
#include <string_view>

#include "camera.h"
#include "engine/data_type.h"
#include "gameobject.hpp"

namespace Vkxel {

    class Scene final : public Object {
    public:
        void Create() override;
        void Start() override;
        void Update() override;
        void Destroy() override;

        std::optional<std::reference_wrapper<GameObject>> GetGameObject(std::string_view gameObjectName);
        std::optional<std::reference_wrapper<GameObject>> GetGameObject(IdType gameObjectId);
        std::ranges::ref_view<std::list<GameObject>> GetGameObjectsView();

        GameObject &CreateGameObject();
        GameObject &AddGameObject(GameObject &&gameObject);

        void DestroyGameObject(const GameObject &gameObject);
        void DestroyGameObject(IdType gameObjectId);
        void DestroyGameObject(std::string_view gameObjectName);

        void SetCamera(Camera &camera);
        std::optional<std::reference_wrapper<Camera>> GetCamera() const;

        void Draw(RenderContext &context) const;

    private:
        void DestroyGameObjectInternal(const std::list<GameObject>::iterator &it);

        std::list<GameObject> _gameobjects;
        std::optional<std::reference_wrapper<Camera>> _mainCamera;
    };

    REGISTER_TYPE(Scene)
    REGISTER_BASE(Object)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_SCENE_H
