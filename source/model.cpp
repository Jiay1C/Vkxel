//
// Created by jiayi on 1/21/2025.
//

#include <array>

#include "model.h"

namespace Vkxel {

   constexpr Model<3,3> ModelLibrary::Triangle = {
      .Index = {0, 1, 2},
      .Vertex = {
         glm::vec3{0.0f, 0.6f, 0.0f},
         glm::vec3{0.6f, -0.3f, 0.0f},
         glm::vec3{-0.6f, -0.3f, 0.0f},
      }
   };

} // Vkxel