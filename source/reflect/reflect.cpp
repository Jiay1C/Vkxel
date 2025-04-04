#include "reflect.hpp"
#include "custom/dual_contouring.h"
#include "custom/gpu_dual_contouring.h"
#include "custom/sdf_surface.h"
#include "world/camera.h"
#include "world/canvas.h"
#include "world/component.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/gameobject.hpp"
#include "world/mesh.h"
#include "world/mover.h"
#include "world/object.h"
#include "world/scene.h"
#include "world/transform.h"

namespace Vkxel {
    void Reflect::Register() {
        Register<Object>();
        Register<Scene>();
        Register<Component>();
        Register<GameObject>();
        Register<Camera>();
        Register<Canvas>();
        Register<Controller>();
        Register<Drawer>();
        Register<Mesh>();
        Register<Mover>();
        Register<Transform>();
        Register<DualContouring>();
        Register<GpuDualContouring>();
        Register<SDFSurface>();
    }

} // namespace Vkxel
