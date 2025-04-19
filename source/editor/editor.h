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
        void SetupDebugUI();
        void SetupSceneUI();
        void SetupInspectorUI();
        void DrawGameObjectTree(GameObject &gameObject);
        void DrawGameObject(GameObject &gameObject);
        void DrawComponent(Component &component);
        void DrawComponentInternal(entt::meta_any &component);
        void DrawElement(std::string_view name, entt::meta_any &element);

        void DrawCreateGameObject(std::optional<std::reference_wrapper<Transform>> parent = std::nullopt);
        void DrawCreateComponent(GameObject &gameObject);

        void DrawString(std::string_view name, std::string &str);
        std::string GetDisplayName(Object &object);

        GameObject *_active_gameobject = nullptr;
        int _selected_component = -1;
    };

} // namespace Vkxel

#endif // VKXEL_EDITOR_H
