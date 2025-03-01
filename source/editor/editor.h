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
        void DrawComponent(Component *component) const;
        void DrawComponentData(entt::meta_any &component) const;
        void DrawElement(std::string_view name, entt::meta_any &element) const;
    };

} // namespace Vkxel

#endif // VKXEL_EDITOR_H
