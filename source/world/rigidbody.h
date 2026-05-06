//
// Created by jiayi on 5/6/2026.
//

#ifndef VKXEL_RIGIDBODY_H
#define VKXEL_RIGIDBODY_H

#include "glm/glm.hpp"

#include "component.h"

namespace Vkxel {

    class Rigidbody;

    enum class RigidbodyMotionType {
        Dynamic,
        Static,
        Kinematic,
    };

    enum class RigidbodyColliderType {
        BoundsBox,
        TriangleMesh,
    };

    REGISTER_TYPE(RigidbodyMotionType)
    REGISTER_ENUM(Dynamic)
    REGISTER_ENUM(Static)
    REGISTER_ENUM(Kinematic)
    REGISTER_END()

    REGISTER_TYPE(RigidbodyColliderType)
    REGISTER_ENUM(BoundsBox)
    REGISTER_ENUM(TriangleMesh)
    REGISTER_END()


    struct RigidbodyConfig {
        RigidbodyMotionType motionType = RigidbodyMotionType::Dynamic;
        RigidbodyColliderType colliderType = RigidbodyColliderType::BoundsBox;
        float mass = 1.0f;
        bool useGravity = true;
        float friction = 0.2f;
        float restitution = 0.0f;
        glm::vec3 linearVelocity = {};
        glm::vec3 angularVelocity = {};
        bool flipTriangleWinding = false;
    };

    REGISTER_TYPE(RigidbodyConfig)
    REGISTER_DATA(motionType)
    REGISTER_DATA(colliderType)
    REGISTER_DATA(mass)
    REGISTER_DATA(useGravity)
    REGISTER_DATA(friction)
    REGISTER_DATA(restitution)
    REGISTER_DATA(linearVelocity)
    REGISTER_DATA(angularVelocity)
    REGISTER_DATA(flipTriangleWinding)
    REGISTER_END()

    class Rigidbody final : public Component {
    public:
        using Component::Component;

        RigidbodyConfig config = {};

        void Simulate(PhysicsContext &context);
    };

    REGISTER_TYPE(Rigidbody)
    REGISTER_BASE(Component)
    REGISTER_DATA(config)
    REGISTER_END()

} // namespace Vkxel

#endif // VKXEL_RIGIDBODY_H
