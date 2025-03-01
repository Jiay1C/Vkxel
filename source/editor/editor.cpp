//
// Created by jiayi on 2/21/2025.
//

#include <format>

#include "editor.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "reflect/reflect.hpp"
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
            ImGui::Text(std::format("Time {0} s", Time::GetSeconds()).data());
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
            for (auto &gameobject: _scene.GetGameObjectsView()) {
                ImGui::PushID(gameobject.id);

                // Name
                if (ImGui::TreeNode(std::format("{0} ({1})", gameobject.name, gameobject.id).data())) {
                    ImGui::InputText(
                            "Name", gameobject.name.data(), gameobject.name.capacity() + 1,
                            ImGuiInputTextFlags_CallbackResize,
                            [](ImGuiInputTextCallbackData *callback_data) {
                                if (callback_data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                                    std::string &callback_str = *static_cast<std::string *>(callback_data->UserData);
                                    callback_str.resize(callback_data->BufTextLen);
                                    callback_data->Buf = callback_str.data();
                                }
                                return 0;
                            },
                            &gameobject.name);

                    // Transform
                    ImGui::PushID(gameobject.transform.id);
                    DrawComponent(&gameobject.transform);
                    ImGui::PopID();

                    // Other Components
                    for (const auto &component: gameobject.GetComponentsView()) {
                        ImGui::PushID(component->id);
                        DrawComponent(component.get());
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

    void EditorEngine::DrawComponent(Component *component) const {
        if (ImGui::TreeNode(std::format("{0} ({1})", component->name, component->id).data())) {
            auto instance = Reflect::GetType(typeid(*component)).from_void(component);

            DrawComponentData(instance);

            if (ImGui::Button("Remove")) {
                component->gameObject.RemoveComponent(*component);
            }

            ImGui::TreePop();
        }
    }

    void EditorEngine::DrawComponentData(entt::meta_any &component) const {
        for (auto &&[id, base]: component.type().base()) {
            auto instance = base.from_void(component.data());
            DrawComponentData(instance);
        }
        for (auto &&[id, elem]: component.type().data()) {
            auto instance = elem.get(component);
            DrawElement(Reflect::GetName(id), instance);
        }
    }


    void EditorEngine::DrawElement(const std::string_view name, entt::meta_any &element) const {
        const auto &type = element.type();
        void *data = element.data();

        if (type.is_integral()) {
            ImGui::DragInt(name.data(), static_cast<int *>(data));
        } else if (type == Reflect::GetType<float>()) {
            ImGui::DragFloat(name.data(), static_cast<float *>(data));
        } else if (type == Reflect::GetType<double>()) {
            ImGui::InputDouble(name.data(), static_cast<double *>(data));
        } else if (type == Reflect::GetType<glm::vec2>()) {
            ImGui::DragFloat2(name.data(), static_cast<float *>(data));
        } else if (type == Reflect::GetType<glm::vec3>()) {
            ImGui::DragFloat3(name.data(), static_cast<float *>(data));
        } else if (type == Reflect::GetType<glm::vec4>()) {
            ImGui::DragFloat4(name.data(), static_cast<float *>(data));
        } else if (type == Reflect::GetType<glm::ivec2>()) {
            ImGui::DragInt2(name.data(), static_cast<int *>(data));
        } else if (type == Reflect::GetType<glm::ivec3>()) {
            ImGui::DragInt3(name.data(), static_cast<int *>(data));
        } else if (type == Reflect::GetType<glm::ivec4>()) {
            ImGui::DragInt4(name.data(), static_cast<int *>(data));
        } else if (type == Reflect::GetType<glm::quat>()) {
            auto &quat = *static_cast<glm::quat *>(data);
            auto deg = glm::degrees(glm::eulerAngles(quat));
            ImGui::DragFloat3(name.data(), reinterpret_cast<float *>(&deg));
            quat = glm::radians(deg);
        } else if (type == Reflect::GetType<std::string>()) {
            auto &str = *static_cast<std::string *>(data);
            ImGui::InputText(
                    name.data(), str.data(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize,
                    [](ImGuiInputTextCallbackData *callback_data) {
                        if (callback_data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                            std::string &callback_str = *static_cast<std::string *>(callback_data->UserData);
                            callback_str.resize(callback_data->BufTextLen);
                            callback_data->Buf = callback_str.data();
                        }
                        return 0;
                    },
                    &str);
        } else if (type.is_enum()) {
            // TODO: Support Enum
            ImGui::Text("%s: Enum <%s>", name.data(), type.info().name().data());
        } else {
            ImGui::Text("%s: Unsupported Type <%s>", name.data(), type.info().name().data());
        }
    }


} // namespace Vkxel
