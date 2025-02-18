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

    enum class PrimitiveType {
        None,
        Sphere,
    };

    enum class CSGType {
        None,
        Union,
        Intersect,
    };

    class SDFSurface final : public Component {
    public:
        using Component::Component;

        SurfaceType surfaceType = SurfaceType::None;

        PrimitiveType primitiveType = PrimitiveType::None;
        CSGType csgType = CSGType::None;
        SDFType customSDF;

        SDFType GetSDF() const;
        SDFOutputType GetSDFValue(SDFInputType p) const;

    private:
        SDFType GetPrimitive() const;
        SDFType GetCSG() const;

        // CSGs
        SDFType Union() const;
        SDFType Intersect() const;

        std::vector<SDFType> GetChildSDF() const;

        // Primitives
        const static SDFType SphereSDF;
        const static SDFType NoneSDF;
    };
} // namespace Vkxel

#endif // VKXEL_SDF_SURFACE_H
