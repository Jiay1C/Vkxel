//
// Created by jiayi on 2/16/2025.
//

#include "mover.h"
#include "engine/vtime.h"
#include "gameobject.hpp"

namespace Vkxel {
    void Mover::Update() {
        float delta_seconds = Time::GetDeltaSeconds();
        gameObject.transform.TranslateSelf(linearVelocity * delta_seconds);
        gameObject.transform.RotateSelf(angularVelocity * delta_seconds);
    }

} // namespace Vkxel
