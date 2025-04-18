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

    enum class SurfaceType : uint32_t {
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

    enum class PrimitiveType : uint32_t {
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

    enum class CSGType : uint32_t {
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

    struct SDFSurfaceGPU {
        glm::mat4 gridToLocal; // 64
        SurfaceType surfaceType; // 4
        PrimitiveType primitiveType; // 4
        CSGType csgType; // 4
        float csgSmoothFactor; // 4
        uint32_t numChildren; // 4
        // ---------84 bytes----------
        uint32_t padding0;
        uint32_t padding1;
        uint32_t padding2;
        // ---------96 bytes----------
    };
    #define MAX_SDF_SURFACE_GPU 32

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
        void BuildSDFTreeGPU(std::vector<SDFSurfaceGPU> &sdf_surface_gpu, const glm::mat4& gridToLocal) const;

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
