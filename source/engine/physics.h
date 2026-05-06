//
// Created by jiayi on 5/6/2026.
//

#ifndef VKXEL_PHYSICS_H
#define VKXEL_PHYSICS_H

#include <memory>

namespace Vkxel {

    class Scene;
    class PhysicsRuntime;

    class Physics final {
    public:
        Physics();
        ~Physics();

        Physics(const Physics &) = delete;
        Physics &operator=(const Physics &) = delete;

        void Step(Scene &scene, float deltaSeconds);

    private:
        std::unique_ptr<PhysicsRuntime> _runtime;
        float _fixed_step_accumulator = 0.0f;
    };

} // namespace Vkxel

#endif // VKXEL_PHYSICS_H
