//
// Created by jiayi on 2/9/2025.
//

#ifndef VKXEL_SDF_SURFACE_H
#define VKXEL_SDF_SURFACE_H

#include <functional>

#include "glm/glm.hpp"
#include "world/component.h"

namespace Vkxel {

    using SDFInputType = const glm::vec3 &;
    using SDFOutputType = float;
    using SDFType = std::function<SDFOutputType(SDFInputType)>;

    enum class SurfaceType {
        None,
        Primitive,
        Custom,
        CSG,
    };

    REGISTER_TYPE(SurfaceType)
    REGISTER_ENUM(None)
    REGISTER_ENUM(Primitive)
    REGISTER_ENUM(Custom)
    REGISTER_ENUM(CSG)
    REGISTER_END()

    enum class PrimitiveType {
        None,
        Sphere,
        Box,
        Capsule,
    };

    REGISTER_TYPE(PrimitiveType)
    REGISTER_ENUM(None)
    REGISTER_ENUM(Sphere)
    REGISTER_ENUM(Box)
    REGISTER_ENUM(Capsule)
    REGISTER_END()

    enum class CSGType {
        None,
        Unionize,
        Intersect,
        Subtract,
    };

    REGISTER_TYPE(CSGType)
    REGISTER_ENUM(None)
    REGISTER_ENUM(Unionize)
    REGISTER_ENUM(Intersect)
    REGISTER_ENUM(Subtract)
    REGISTER_END()

    class SDFSurface final : public Component {
    public:
        using Component::Component;

        SurfaceType surfaceType = SurfaceType::None;

        PrimitiveType primitiveType = PrimitiveType::None;

        CSGType csgType = CSGType::None;
        float csgSmoothFactor = 0.0f;

        SDFType customSDF;

        SDFType GetSDF() const;
        SDFOutputType GetSDFValue(SDFInputType p) const;

    private:
        SDFType GetPrimitive() const;
        SDFType GetCSG() const;

        // CSGs
        SDFType Unionize() const;
        SDFType Intersect() const;
        SDFType Subtract() const;

        std::vector<SDFType> GetChildSDF() const;

        // Primitives
        const static SDFType SphereSDF;
        const static SDFType BoxSDF;
        const static SDFType CapsuleSDF;
        const static SDFType NoneSDF;
    };

    REGISTER_TYPE(SDFSurface)
    REGISTER_BASE(Component)
    REGISTER_DATA(surfaceType)
    REGISTER_DATA(primitiveType)
    REGISTER_DATA(csgType)
    REGISTER_DATA(csgSmoothFactor)
    REGISTER_END()
} // namespace Vkxel

#endif // VKXEL_SDF_SURFACE_H
