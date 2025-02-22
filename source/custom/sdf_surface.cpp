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
            case PrimitiveType::Box:
                return BoxSDF;
            case PrimitiveType::Capsule:
                return CapsuleSDF;
            default:
                return NoneSDF;
        }
    }

    SDFType SDFSurface::GetCSG() const {
        switch (csgType) {
            case CSGType::Unionize:
                return Unionize();
            case CSGType::Intersect:
                return Intersect();
            case CSGType::Subtract:
                return Subtract();
            default:
                return NoneSDF;
        }
    }

    SDFType SDFSurface::Unionize() const {
        const auto child_sdf = GetChildSDF();

        if (csgSmoothFactor <= 0.0f) { // Union
            return [=](SDFInputType p) {
                SDFOutputType value = std::numeric_limits<SDFOutputType>::max();
                for (const auto &sdf: child_sdf) {
                    value = std::min(value, sdf(p));
                }
                return value;
            };
        }

        // Smooth Union
        return [=](SDFInputType p) {
            SDFOutputType value = std::numeric_limits<SDFOutputType>::max();
            for (const auto &sdf: child_sdf) {
                float sdf_value = sdf(p);
                float h = glm::clamp(0.5f + 0.5f * (sdf_value - value) / csgSmoothFactor, 0.0f, 1.0f);
                value = glm::mix(sdf_value, value, h) - csgSmoothFactor * h * (1.0f - h);
            }
            return value;
        };
    }

    SDFType SDFSurface::Intersect() const {
        const auto child_sdf = GetChildSDF();

        // Intersect
        if (csgSmoothFactor <= 0.0f) {
            return [=](SDFInputType p) {
                SDFOutputType value = std::numeric_limits<SDFOutputType>::lowest();
                for (const auto &sdf: child_sdf) {
                    value = std::max(value, sdf(p));
                }
                return value;
            };
        }

        // Smooth Intersect
        return [=](SDFInputType p) {
            SDFOutputType value = std::numeric_limits<SDFOutputType>::lowest();
            for (const auto &sdf: child_sdf) {
                float sdf_value = sdf(p);
                float h = glm::clamp(0.5f - 0.5f * (sdf_value - value) / csgSmoothFactor, 0.0f, 1.0f);
                value = glm::mix(sdf_value, value, h) + csgSmoothFactor * h * (1.0f - h);
            }
            return value;
        };
    }

    SDFType SDFSurface::Subtract() const {
        const auto child_sdf = GetChildSDF();

        // Subtract
        if (csgSmoothFactor <= 0.0f) {
            return [=](SDFInputType p) {
                SDFOutputType value = std::numeric_limits<SDFOutputType>::max();
                for (bool first = true; const auto &sdf: child_sdf) {
                    if (first) {
                        value = sdf(p);
                    } else {
                        value = std::max(value, -sdf(p));
                    }
                    first = false;
                }
                return value;
            };
        }

        // Smooth Subtract
        return [=](SDFInputType p) {
            SDFOutputType value = std::numeric_limits<SDFOutputType>::max();
            for (bool first = true; const auto &sdf: child_sdf) {
                if (first) {
                    value = sdf(p);
                } else {
                    float sdf_value = sdf(p);
                    float h = glm::clamp(0.5f - 0.5f * (value + sdf_value) / csgSmoothFactor, 0.0f, 1.0f);
                    value = glm::mix(value, -sdf_value, h) + csgSmoothFactor * h * (1.0f - h);
                }
                first = false;
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

    const SDFType SDFSurface::BoxSDF = [](SDFInputType p) {
        glm::vec3 q = glm::abs(p) - glm::vec3{1, 1, 1};
        return glm::length(glm::max(q, 0.0f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
    };

    const SDFType SDFSurface::CapsuleSDF = [](SDFInputType p) {
        glm::vec3 q = p;
        q.y -= glm::clamp(q.y, -0.5f, 0.5f);
        return glm::length(q) - 0.5f;
    };

    const SDFType SDFSurface::NoneSDF = [](SDFInputType p) { return std::numeric_limits<float>::max(); };

} // namespace Vkxel
