//
// Created by jiayi on 2/5/2025.
//

#ifndef VKXEL_DRAWER_H
#define VKXEL_DRAWER_H

#include "component.h"
#include "engine/data_type.h"

namespace Vkxel {

    class Drawer final : public Component {
    public:
        using Component::Component;

        MaterialData material = {};

        void Draw(RenderContext &context) const;
    };

} // namespace Vkxel

#endif // VKXEL_DRAWER_H
