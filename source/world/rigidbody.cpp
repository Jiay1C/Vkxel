//
// Created by jiayi on 5/6/2026.
//

#include "rigidbody.h"

#include "gameobject.hpp"
#include "mesh.h"

namespace Vkxel {

    void Rigidbody::Simulate(PhysicsContext &context) { context.rigidbodies.emplace_back(*this); }

} // namespace Vkxel
