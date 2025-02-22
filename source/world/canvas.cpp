//
// Created by jiayi on 2/22/2025.
//

#include "canvas.h"
#include "gameobject.hpp"

namespace Vkxel {

    void Canvas::OnGUI() const {
        if (ImGui::CollapsingHeader(gameObject.name.data())) {
            uiItems();
        }
    }

} // namespace Vkxel
