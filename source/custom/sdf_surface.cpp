//
// Created by jiayi on 2/9/2025.
//

#include "sdf_surface.h"

#include "world/gameobject.hpp"
namespace Vkxel {
    SDFType SDFSurface::GetSDF() const {
        switch (surfaceType) {
            case SurfaceType::Primitive:
                return GetPrimitive();
            case SurfaceType::Custom:
                return customSDF;
            case SurfaceType::CSG:
                return GetCSG();
            default:
                return NoneSDF;
        }
    }

    SDFOutputType SDFSurface::GetSDFValue(SDFInputType p) const { return GetSDF()(p); }

    SDFType SDFSurface::GetPrimitive() const {
        switch (primitiveType) {
            case PrimitiveType::Sphere:
                return SphereSDF;
            default:
                return NoneSDF;
        }
    }

    SDFType SDFSurface::GetCSG() const {
        switch (csgType) {
            case CSGType::Intersect:
                return Intersect();
            case CSGType::Union:
                return Union();
            default:
                return NoneSDF;
        }
    }

    SDFType SDFSurface::Union() const {
        const auto child_sdf = GetChildSDF();

        return [=](SDFInputType p) {
            SDFOutputType value = std::numeric_limits<SDFOutputType>::max();
            for (const auto &sdf: child_sdf) {
                value = std::min(value, sdf(p));
            }
            return value;
        };
    }

    SDFType SDFSurface::Intersect() const {
        const auto child_sdf = GetChildSDF();

        return [=](SDFInputType p) {
            SDFOutputType value = std::numeric_limits<SDFOutputType>::lowest();
            for (const auto &sdf: child_sdf) {
                value = std::max(value, sdf(p));
            }
            return value;
        };
    }

    std::vector<SDFType> SDFSurface::GetChildSDF() const {
        std::vector<SDFType> child_sdf;
        for (const auto &child_wrapper: gameObject.transform.GetChildren()) {
            Transform &child = child_wrapper;
            if (auto child_sdf_surface = child.gameObject.GetComponent<SDFSurface>()) {
                glm::mat4 child_transform = child.GetRelativeToLocalMatrix();
                // TODO: Might Affect Dual Contouring When Using Non-Uniform Scaling
                float child_minimum_scale = std::min({child.scale.x, child.scale.y, child.scale.z});
                SDFType sdf = child_sdf_surface.value().get().GetSDF();
                child_sdf.emplace_back([=](SDFInputType p) {
                    return child_minimum_scale * sdf(child_transform * glm::vec4{p, 1.0f});
                });
            }
        }
        return child_sdf;
    }

    const SDFType SDFSurface::SphereSDF = [](SDFInputType p) { return glm::length(p) - 1; };

    const SDFType SDFSurface::NoneSDF = [](SDFInputType p) { return std::numeric_limits<float>::max(); };

} // namespace Vkxel
