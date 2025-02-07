//
// Created by jiayi on 2/6/2025.
//

#ifndef VKXEL_SCENE_H
#define VKXEL_SCENE_H

#include <list>
#include <optional>
#include <string_view>

#include "engine/data_type.h"
#include "gameobject.hpp"

namespace Vkxel {

    class Scene final : public Object {
    public:
        void Create() override;
        void Update() override;
        void Destroy() override;

        GameObject &CreateGameObject();
        GameObject &AddGameObject(GameObject &&gameObject);
        void DestroyGameObject(std::string_view name);

        void SetCamera(std::string_view name);
        std::optional<std::reference_wrapper<const GameObject>> GetCamera() const;

        void Draw(RenderContext &context) const;

    private:
        std::list<GameObject> _gameobjects;
        std::optional<std::reference_wrapper<GameObject>> _mainCamera;
    };

} // namespace Vkxel

#endif // VKXEL_SCENE_H
