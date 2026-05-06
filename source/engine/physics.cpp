//
// Created by jiayi on 5/6/2026.
//

#include "physics.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

#include "Jolt/Jolt.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Geometry/IndexedTriangle.h"
#include "Jolt/Math/Float3.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Body/MassProperties.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h"
#include "Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h"
#include "Jolt/Physics/Collision/ObjectLayerPairFilterTable.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/EActivation.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/RegisterTypes.h"

#include "world/gameobject.hpp"
#include "world/mesh.h"
#include "world/rigidbody.h"
#include "world/scene.h"
#include "world/transform.h"
#include "util/debug.hpp"

namespace Vkxel {

    namespace {

        namespace PhysicsLayers {
            constexpr JPH::ObjectLayer NonMoving = 0;
            constexpr JPH::ObjectLayer Moving = 1;
            constexpr JPH::ObjectLayer Count = 2;

            constexpr JPH::BroadPhaseLayer BroadPhaseNonMoving(0);
            constexpr JPH::BroadPhaseLayer BroadPhaseMoving(1);
            constexpr uint32_t BroadPhaseCount = 2;
        } // namespace PhysicsLayers

        constexpr uint32_t max_bodies = 65536;
        constexpr uint32_t body_mutex_count = 0;
        constexpr uint32_t max_body_pairs = 65536;
        constexpr uint32_t max_contact_constraints = 10240;
        constexpr uint32_t temp_allocator_size = 64 * 1024 * 1024;
        constexpr float fixed_delta_seconds = 1.0f / 60.0f;
        constexpr float max_frame_delta_seconds = 0.25f;
        constexpr int max_physics_steps_per_frame = 4;
        constexpr int collision_steps = 1;
        constexpr float min_extent = 0.001f;
        constexpr float transform_epsilon = 0.0001f;

        uint32_t physics_world_count = 0;

        struct BodyResource {
            JPH::BodyID bodyId;
            RigidbodyConfig config = {};
            uint64_t meshRevision = 0;
            glm::vec3 scale = {};
            glm::vec3 position = {};
            glm::quat rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
        };

        void TraceImpl(const char *format, ...) {
            char buffer[1024];

            va_list list;
            va_start(list, format);
            vsnprintf(buffer, sizeof(buffer), format, list);
            va_end(list);

            Debug::Log("Jolt: {}", buffer);
        }

#ifdef JPH_ENABLE_ASSERTS
        bool AssertFailedImpl(const char *expression, const char *message, const char *file, JPH::uint line) {
            Debug::LogError("{}:{}: ({}) {}", file, line, expression, message != nullptr ? message : "");
            return true;
        }
#endif

        void InitializeJoltRuntime() {
            if (physics_world_count++ != 0) {
                return;
            }

            JPH::RegisterDefaultAllocator();
            JPH::Trace = TraceImpl;
            JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

            JPH::Factory::sInstance = new JPH::Factory();
            JPH::RegisterTypes();
        }

        void ShutdownJoltRuntime() {
            if (--physics_world_count != 0) {
                return;
            }

            JPH::UnregisterTypes();
            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;
        }

        JPH::EMotionType ToJoltMotionType(const RigidbodyMotionType motionType) {
            switch (motionType) {
                case RigidbodyMotionType::Static:
                    return JPH::EMotionType::Static;
                case RigidbodyMotionType::Kinematic:
                    return JPH::EMotionType::Kinematic;
                case RigidbodyMotionType::Dynamic:
                default:
                    return JPH::EMotionType::Dynamic;
            }
        }

        JPH::ObjectLayer ToJoltObjectLayer(const RigidbodyMotionType motionType) {
            return motionType == RigidbodyMotionType::Static ? PhysicsLayers::NonMoving : PhysicsLayers::Moving;
        }

        JPH::RVec3 ToJoltPosition(const glm::vec3 &value) { return {value.x, value.y, value.z}; }

        JPH::Vec3 ToJoltVector(const glm::vec3 &value) { return {value.x, value.y, value.z}; }

        JPH::Quat ToJoltRotation(const glm::quat &value) {
            const glm::quat normalized = glm::normalize(value);
            return JPH::Quat(normalized.x, normalized.y, normalized.z, normalized.w);
        }

        template<typename Type>
        glm::vec3 ToGlmVector(const Type &value) {
            return {static_cast<float>(value.GetX()), static_cast<float>(value.GetY()),
                    static_cast<float>(value.GetZ())};
        }

        glm::quat ToGlmRotation(const JPH::Quat &value) {
            return glm::normalize(glm::quat(value.GetW(), value.GetX(), value.GetY(), value.GetZ()));
        }

        bool ApproximatelyEqual(const glm::vec3 &lhs, const glm::vec3 &rhs) {
            return glm::length(lhs - rhs) <= transform_epsilon;
        }

        bool ApproximatelyEqual(const glm::quat &lhs, const glm::quat &rhs) {
            return std::abs(glm::dot(glm::normalize(lhs), glm::normalize(rhs))) >= 1.0f - transform_epsilon;
        }

        glm::vec3 SafeScale(const glm::vec3 &scale) {
            return {scale.x == 0.0f ? min_extent : scale.x, scale.y == 0.0f ? min_extent : scale.y,
                    scale.z == 0.0f ? min_extent : scale.z};
        }

        glm::vec3 SafeHalfExtent(const glm::vec3 &extent) {
            return {std::max(min_extent, extent.x), std::max(min_extent, extent.y), std::max(min_extent, extent.z)};
        }

        const Mesh *GetMesh(const Rigidbody &rigidbody) {
            if (const auto mesh_result = rigidbody.gameObject.GetComponent<Mesh>()) {
                return &mesh_result->get();
            }
            return nullptr;
        }

        MeshBounds GetMeshBounds(const Rigidbody &rigidbody) {
            if (const Mesh *mesh = GetMesh(rigidbody)) {
                return mesh->GetBounds();
            }
            return {.min = glm::vec3{-0.5f}, .max = glm::vec3{0.5f}};
        }

        uint64_t GetMeshRevision(const Rigidbody &rigidbody) {
            if (const Mesh *mesh = GetMesh(rigidbody)) {
                return mesh->GetRevision();
            }
            return 0;
        }

        const MeshData *GetMeshData(const Rigidbody &rigidbody) {
            if (const Mesh *mesh = GetMesh(rigidbody)) {
                if (const auto &data = mesh->GetMesh()) {
                    return &data.value();
                }
            }
            return nullptr;
        }

        JPH::ShapeRefC CreateBoundsBoxShape(const Rigidbody &rigidbody) {
            const MeshBounds mesh_bounds = GetMeshBounds(rigidbody);
            const glm::vec3 scale = SafeScale(rigidbody.gameObject.transform.GetWorldScale());
            const glm::vec3 half_extent =
                    SafeHalfExtent((mesh_bounds.max - mesh_bounds.min) * glm::abs(scale) * 0.5f);
            const glm::vec3 center = (mesh_bounds.max + mesh_bounds.min) * scale * 0.5f;

            JPH::ShapeRefC box = new JPH::BoxShape(ToJoltVector(half_extent), 0.0f);
            if (ApproximatelyEqual(center, glm::vec3{})) {
                return box;
            }

            return new JPH::RotatedTranslatedShape(ToJoltVector(center), JPH::Quat::sIdentity(), box);
        }

        JPH::ShapeRefC CreateTriangleMeshShape(const Rigidbody &rigidbody) {
            const MeshData *mesh_data = GetMeshData(rigidbody);
            if (mesh_data == nullptr) {
                return nullptr;
            }

            const auto *cpu_mesh = std::get_if<CPUMeshData>(mesh_data);
            if (cpu_mesh == nullptr || cpu_mesh->vertex.empty() || cpu_mesh->index.size() < 3) {
                return nullptr;
            }

            const glm::vec3 scale = SafeScale(rigidbody.gameObject.transform.GetWorldScale());
            const bool should_flip_winding =
                    rigidbody.config.flipTriangleWinding != (scale.x * scale.y * scale.z < 0.0f);

            JPH::VertexList vertices;
            vertices.reserve(static_cast<JPH::uint>(cpu_mesh->vertex.size()));
            for (const auto &vertex: cpu_mesh->vertex) {
                const glm::vec3 position = vertex.position * scale;
                vertices.push_back(JPH::Float3(position.x, position.y, position.z));
            }

            JPH::IndexedTriangleList triangles;
            triangles.reserve(static_cast<JPH::uint>(cpu_mesh->index.size() / 3));
            for (size_t index = 0; index + 2 < cpu_mesh->index.size(); index += 3) {
                const IndexType i0 = cpu_mesh->index[index];
                const IndexType i1 = cpu_mesh->index[index + 1];
                const IndexType i2 = cpu_mesh->index[index + 2];
                if (i0 >= cpu_mesh->vertex.size() || i1 >= cpu_mesh->vertex.size() || i2 >= cpu_mesh->vertex.size()) {
                    continue;
                }

                if (should_flip_winding) {
                    triangles.push_back(JPH::IndexedTriangle(i0, i2, i1));
                } else {
                    triangles.push_back(JPH::IndexedTriangle(i0, i1, i2));
                }
            }

            if (triangles.empty()) {
                return nullptr;
            }

            JPH::MeshShapeSettings settings(std::move(vertices), std::move(triangles));
            const JPH::ShapeSettings::ShapeResult result = settings.Create();
            if (result.HasError()) {
                Debug::LogWarning("Failed to create triangle mesh collider for '{}': {}", rigidbody.gameObject.name,
                                  result.GetError());
                return nullptr;
            }

            return result.Get();
        }

        JPH::ShapeRefC CreateShape(const Rigidbody &rigidbody) {
            if (rigidbody.config.colliderType == RigidbodyColliderType::TriangleMesh &&
                rigidbody.config.motionType != RigidbodyMotionType::Dynamic) {
                if (JPH::ShapeRefC triangle_mesh = CreateTriangleMeshShape(rigidbody)) {
                    return triangle_mesh;
                }
            }

            return CreateBoundsBoxShape(rigidbody);
        }

        bool NeedsRebuild(const Rigidbody &rigidbody, const BodyResource &resource) {
            const RigidbodyConfig &config = rigidbody.config;
            return config.motionType != resource.config.motionType ||
                   config.colliderType != resource.config.colliderType ||
                   GetMeshRevision(rigidbody) != resource.meshRevision ||
                   !ApproximatelyEqual(rigidbody.gameObject.transform.GetWorldScale(), resource.scale) ||
                   config.mass != resource.config.mass ||
                   config.flipTriangleWinding != resource.config.flipTriangleWinding;
        }

        void StoreResourceSnapshot(const Rigidbody &rigidbody, BodyResource &resource) {
            resource.config = rigidbody.config;
            resource.meshRevision = GetMeshRevision(rigidbody);
            resource.scale = rigidbody.gameObject.transform.GetWorldScale();
            resource.position = rigidbody.gameObject.transform.GetWorldPosition();
            resource.rotation = rigidbody.gameObject.transform.GetWorldRotation();
        }

    } // namespace

    class PhysicsRuntime final {
    public:
        PhysicsRuntime() {
            InitializeJoltRuntime();

            broadPhaseLayerInterface = std::make_unique<JPH::BroadPhaseLayerInterfaceTable>(
                    PhysicsLayers::Count, PhysicsLayers::BroadPhaseCount);
            broadPhaseLayerInterface->MapObjectToBroadPhaseLayer(PhysicsLayers::NonMoving,
                                                                 PhysicsLayers::BroadPhaseNonMoving);
            broadPhaseLayerInterface->MapObjectToBroadPhaseLayer(PhysicsLayers::Moving,
                                                                 PhysicsLayers::BroadPhaseMoving);

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            broadPhaseLayerInterface->SetBroadPhaseLayerName(PhysicsLayers::BroadPhaseNonMoving, "NonMoving");
            broadPhaseLayerInterface->SetBroadPhaseLayerName(PhysicsLayers::BroadPhaseMoving, "Moving");
#endif

            objectLayerPairFilter = std::make_unique<JPH::ObjectLayerPairFilterTable>(PhysicsLayers::Count);
            objectLayerPairFilter->EnableCollision(PhysicsLayers::NonMoving, PhysicsLayers::Moving);
            objectLayerPairFilter->EnableCollision(PhysicsLayers::Moving, PhysicsLayers::Moving);

            objectVsBroadPhaseLayerFilter = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
                    *broadPhaseLayerInterface, PhysicsLayers::BroadPhaseCount, *objectLayerPairFilter,
                    PhysicsLayers::Count);

            physicsSystem = std::make_unique<JPH::PhysicsSystem>();
            physicsSystem->Init(max_bodies, body_mutex_count, max_body_pairs, max_contact_constraints,
                                *broadPhaseLayerInterface, *objectVsBroadPhaseLayerFilter, *objectLayerPairFilter);

            tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(temp_allocator_size);

            const uint32_t thread_count = std::max(1u, std::thread::hardware_concurrency());
            jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                                   std::max(1u, thread_count - 1));
        }

        ~PhysicsRuntime() {
            DestroyAllBodies();

            physicsSystem.reset();
            jobSystem.reset();
            tempAllocator.reset();
            objectVsBroadPhaseLayerFilter.reset();
            objectLayerPairFilter.reset();
            broadPhaseLayerInterface.reset();

            ShutdownJoltRuntime();
        }

        JPH::BodyInterface &GetBodyInterface() { return physicsSystem->GetBodyInterface(); }

        void DestroyBody(const BodyResource &resource) {
            if (resource.bodyId.IsInvalid()) {
                return;
            }

            JPH::BodyInterface &body_interface = GetBodyInterface();
            body_interface.RemoveBody(resource.bodyId);
            body_interface.DestroyBody(resource.bodyId);
        }

        void DestroyAllBodies() {
            for (const auto &entry: bodyResources) {
                DestroyBody(entry.second);
            }
            bodyResources.clear();
        }

        std::unique_ptr<JPH::BroadPhaseLayerInterfaceTable> broadPhaseLayerInterface;
        std::unique_ptr<JPH::ObjectLayerPairFilterTable> objectLayerPairFilter;
        std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> objectVsBroadPhaseLayerFilter;
        std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
        std::unordered_map<IdType, BodyResource> bodyResources;
    };

    Physics::Physics() : _runtime(std::make_unique<PhysicsRuntime>()) {}

    Physics::~Physics() = default;

    void Physics::Step(Scene &scene, const float deltaSeconds) {
        PhysicsContext context;
        scene.Simulate(context);

        std::unordered_set<IdType> active_body_ids;
        active_body_ids.reserve(context.rigidbodies.size());

        JPH::BodyInterface &body_interface = _runtime->GetBodyInterface();

        for (const std::reference_wrapper<Rigidbody> &rigidbody_ref: context.rigidbodies) {
            Rigidbody &rigidbody = rigidbody_ref.get();
            RigidbodyConfig &config = rigidbody.config;
            Transform &transform = rigidbody.gameObject.transform;
            active_body_ids.insert(rigidbody.id);

            auto body_resource_it = _runtime->bodyResources.find(rigidbody.id);
            if (body_resource_it == _runtime->bodyResources.end() || NeedsRebuild(rigidbody, body_resource_it->second)) {
                if (body_resource_it != _runtime->bodyResources.end()) {
                    _runtime->DestroyBody(body_resource_it->second);
                    _runtime->bodyResources.erase(body_resource_it);
                }

                const JPH::ShapeRefC shape = CreateShape(rigidbody);
                if (!shape) {
                    continue;
                }

                JPH::BodyCreationSettings settings(shape, ToJoltPosition(transform.GetWorldPosition()),
                                                   ToJoltRotation(transform.GetWorldRotation()),
                                                   ToJoltMotionType(config.motionType),
                                                   ToJoltObjectLayer(config.motionType));
                settings.mFriction = config.friction;
                settings.mRestitution = config.restitution;
                settings.mLinearVelocity = ToJoltVector(config.linearVelocity);
                settings.mAngularVelocity = ToJoltVector(config.angularVelocity);
                settings.mGravityFactor = config.useGravity ? 1.0f : 0.0f;
                settings.mEnhancedInternalEdgeRemoval = true;
                settings.mUserData = rigidbody.gameObject.id;

                if (config.motionType != RigidbodyMotionType::Static) {
                    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
                    settings.mMassPropertiesOverride.mMass = std::max(min_extent, config.mass);
                }

                const JPH::EActivation activation = config.motionType == RigidbodyMotionType::Dynamic
                                                            ? JPH::EActivation::Activate
                                                            : JPH::EActivation::DontActivate;
                const JPH::BodyID body_id = body_interface.CreateAndAddBody(settings, activation);
                if (body_id.IsInvalid()) {
                    Debug::LogWarning("Failed to create rigidbody for '{}'", rigidbody.gameObject.name);
                    continue;
                }

                BodyResource resource = {.bodyId = body_id};
                StoreResourceSnapshot(rigidbody, resource);
                body_resource_it = _runtime->bodyResources.emplace(rigidbody.id, resource).first;
            }

            BodyResource &resource = body_resource_it->second;
            const JPH::BodyID body_id = resource.bodyId;

            if (config.friction != resource.config.friction) {
                body_interface.SetFriction(body_id, config.friction);
                resource.config.friction = config.friction;
            }
            if (config.restitution != resource.config.restitution) {
                body_interface.SetRestitution(body_id, config.restitution);
                resource.config.restitution = config.restitution;
            }
            if (config.motionType != RigidbodyMotionType::Static && config.useGravity != resource.config.useGravity) {
                body_interface.SetGravityFactor(body_id, config.useGravity ? 1.0f : 0.0f);
            }
            resource.config.useGravity = config.useGravity;

            const glm::vec3 world_position = transform.GetWorldPosition();
            const glm::quat world_rotation = transform.GetWorldRotation();

            switch (config.motionType) {
                case RigidbodyMotionType::Static:
                    body_interface.SetPositionAndRotationWhenChanged(body_id, ToJoltPosition(world_position),
                                                                      ToJoltRotation(world_rotation),
                                                                      JPH::EActivation::DontActivate);
                    resource.position = world_position;
                    resource.rotation = world_rotation;
                    break;
                case RigidbodyMotionType::Kinematic:
                    if (deltaSeconds > 0.0f) {
                        body_interface.MoveKinematic(body_id, ToJoltPosition(world_position),
                                                     ToJoltRotation(world_rotation), deltaSeconds);
                    } else {
                        body_interface.SetPositionAndRotationWhenChanged(body_id, ToJoltPosition(world_position),
                                                                          ToJoltRotation(world_rotation),
                                                                          JPH::EActivation::Activate);
                    }
                    resource.position = world_position;
                    resource.rotation = world_rotation;
                    break;
                case RigidbodyMotionType::Dynamic:
                    if (!ApproximatelyEqual(world_position, resource.position) ||
                        !ApproximatelyEqual(world_rotation, resource.rotation)) {
                        body_interface.SetPositionAndRotationWhenChanged(body_id, ToJoltPosition(world_position),
                                                                          ToJoltRotation(world_rotation),
                                                                          JPH::EActivation::Activate);
                        resource.position = world_position;
                        resource.rotation = world_rotation;
                    }
                    if (!ApproximatelyEqual(config.linearVelocity, resource.config.linearVelocity)) {
                        body_interface.SetLinearVelocity(body_id, ToJoltVector(config.linearVelocity));
                        resource.config.linearVelocity = config.linearVelocity;
                    }
                    if (!ApproximatelyEqual(config.angularVelocity, resource.config.angularVelocity)) {
                        body_interface.SetAngularVelocity(body_id, ToJoltVector(config.angularVelocity));
                        resource.config.angularVelocity = config.angularVelocity;
                    }
                    break;
            }
        }

        for (auto it = _runtime->bodyResources.begin(); it != _runtime->bodyResources.end();) {
            if (!active_body_ids.contains(it->first)) {
                _runtime->DestroyBody(it->second);
                it = _runtime->bodyResources.erase(it);
            } else {
                ++it;
            }
        }

        if (deltaSeconds > 0) {
            _fixed_step_accumulator = std::min(_fixed_step_accumulator + std::min(deltaSeconds, max_frame_delta_seconds),
                                               fixed_delta_seconds * max_physics_steps_per_frame);

            while (_fixed_step_accumulator >= fixed_delta_seconds) {
                _runtime->physicsSystem->Update(fixed_delta_seconds, collision_steps, _runtime->tempAllocator.get(),
                                                _runtime->jobSystem.get());
                _fixed_step_accumulator -= fixed_delta_seconds;
            }
        }

        for (const auto &rigidbody_ref: context.rigidbodies) {
            Rigidbody &rigidbody = rigidbody_ref.get();
            RigidbodyConfig &config = rigidbody.config;
            if (config.motionType != RigidbodyMotionType::Dynamic) {
                continue;
            }

            const auto body_resource_it = _runtime->bodyResources.find(rigidbody.id);
            if (body_resource_it == _runtime->bodyResources.end()) {
                continue;
            }

            BodyResource &resource = body_resource_it->second;

            JPH::RVec3 position;
            JPH::Quat rotation;
            body_interface.GetPositionAndRotation(resource.bodyId, position, rotation);

            resource.position = ToGlmVector(position);
            resource.rotation = ToGlmRotation(rotation);
            rigidbody.gameObject.transform.SetWorldPosition(resource.position);
            rigidbody.gameObject.transform.SetWorldRotation(resource.rotation);

            config.linearVelocity = ToGlmVector(body_interface.GetLinearVelocity(resource.bodyId));
            config.angularVelocity = ToGlmVector(body_interface.GetAngularVelocity(resource.bodyId));
            resource.config.linearVelocity = config.linearVelocity;
            resource.config.angularVelocity = config.angularVelocity;
        }
    }

} // namespace Vkxel
