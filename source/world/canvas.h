//
// Created by jiayi on 2/22/2025.
//

#ifndef VKXEL_CANVAS_H
#define VKXEL_CANVAS_H

#include "component.h"
#include "engine/gui.h"

namespace Vkxel {

    class Canvas final : public Component {
    public:
        using Component::Component;

        GUI::GuiDelegate uiItems;

        virtual void OnGUI() const;
    };

    REGISTER_TYPE(Canvas)
    REGISTER_BASE(Component)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_CANVAS_H
