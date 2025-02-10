//
// Created by jiayi on 2/9/2025.
//

#ifndef VKXEL_SDF_H
#define VKXEL_SDF_H

#include <functional>

#include "glm/glm.hpp"

namespace Vkxel {

    using SDF = std::function<float(const glm::vec3 &)>;

    class SDFLibrary {
    public:
        static const SDF StanfordBunny;
    };

} // namespace Vkxel

#endif // VKXEL_SDF_H
