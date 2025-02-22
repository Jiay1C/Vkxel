//
// Created by jiayi on 2/21/2025.
//

#include <format>

#include "editor.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "world/camera.h"
#include "world/controller.h"

namespace Vkxel {

    EditorEngine::EditorEngine(Scene &scene) : Engine(scene) {
        SetupDebugUI();
        SetupSceneUI();
    }

    void EditorEngine::SetupDebugUI() const {
        _gui->AddItem("Debug", [&]() {
            ImGui::Text(std::format("Frame {0} ({1} ms)", Time::GetTicks(), Time::GetRealDeltaSeconds() * 1000).data());
            ImGui::Text(std::format("Size ({0}, {1})", _window->GetWidth(), _window->GetHeight()).data());
            ImGui::Text(std::format("Resolution ({0}, {1})", _window->GetFrameBufferWidth(),
                                    _window->GetFrameBufferHeight())
                                .data());

            if (auto camera_result = _scene.GetCamera()) {
                Camera &camera = camera_result.value();
                if (ImGui::CollapsingHeader("Camera")) {
                    ImGui::DragFloat("Near Clip Plane", &camera.nearClipPlane);
                    ImGui::DragFloat("Far Clip Plane", &camera.farClipPlane);

                    auto fov = glm::degrees(camera.fieldOfViewY);
                    ImGui::DragFloat("Field of View", &fov);
                    camera.fieldOfViewY = glm::radians(fov);
                }

                if (auto controller_result = camera.gameObject.GetComponent<Controller>()) {
                    Controller &camera_controller = controller_result.value();
                    if (ImGui::CollapsingHeader("Controller")) {
                        ImGui::DragFloat("Move Speed", &camera_controller.moveSpeed, 0.02f, 0.0f, 10.0f);
                        ImGui::DragFloat("Rotate Speed", &camera_controller.rotateSpeed, 0.02f, 0.0f, 10.0f);
                        ImGui::DragFloat("Accelerate Ratio", &camera_controller.accelerateRatio, 0.02f, 0.0f, 10.0f);
                    }
                }
            }
        });
    }

    void EditorEngine::SetupSceneUI() const {
        _gui->AddItem("Scene", [&]() {
            for (auto &gameobject: _scene.GetGameObjectList()) {
                ImGui::PushID(gameobject.id);
                if (ImGui::TreeNode(std::format("{0} ({1})", gameobject.name, gameobject.id).data())) {
                    // Transform
                    if (Transform &transform = gameobject.transform;
                        ImGui::TreeNode(std::format("{0} ({1})", transform.name, transform.id).data())) {
                        auto &position = transform.position;
                        ImGui::DragFloat3("Position", reinterpret_cast<float *>(&position));

                        auto rotation = glm::degrees(glm::eulerAngles(transform.rotation));
                        ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&rotation));
                        transform.rotation = glm::radians(rotation);

                        auto &scale = transform.scale;
                        ImGui::DragFloat3("Scale", reinterpret_cast<float *>(&scale));

                        ImGui::TreePop();
                    }

                    // Other Components
                    for (const auto &component: gameobject.GetComponentList()) {
                        ImGui::PushID(component->id);
                        if (ImGui::TreeNode(std::format("{0} ({1})", component->name, component->id).data())) {
                            // TODO: Fix Crash When Remove Component
                            // if (ImGui::Button("Remove")) {
                            //     gameobject.RemoveComponent(*component);
                            // }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }

                    if (ImGui::Button("Destroy")) {
                        _scene.DestroyGameObject(gameobject);
                    }

                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        });
    }


} // namespace Vkxel
