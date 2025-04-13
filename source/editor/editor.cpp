//
// Created by jiayi on 2/21/2025.
//

#include <format>

#include "editor.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "reflect/reflect.hpp"

namespace Vkxel {

    EditorEngine::EditorEngine(Scene &scene) : Engine(scene) {
        SetupDebugUI();
        SetupSceneUI();
        SetupInspectorUI();
    }

    void EditorEngine::SetupDebugUI() {
        _gui->AddItem("Debug", [&]() {
            ImGui::Text(std::format("Frame {0} ({1} ms)", Time::GetTicks(), Time::GetRealDeltaSeconds() * 1000).data());
            ImGui::Text(std::format("Time {0} s", Time::GetSeconds()).data());
            ImGui::Text(std::format("Size ({0}, {1})", _window->GetWidth(), _window->GetHeight()).data());
            ImGui::Text(std::format("Resolution ({0}, {1})", _window->GetFrameBufferWidth(),
                                    _window->GetFrameBufferHeight())
                                .data());
        });
    }

    void EditorEngine::SetupSceneUI() {
        _gui->AddItem("Scene", [&]() {
            for (auto &gameobject: _scene.GetGameObjectsView()) {
                ImGui::PushID(gameobject.id);

                // Name
                if (ImGui::TreeNode(GetDisplayName(gameobject).data())) {
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
                    ImGui::Bullet();
                    if (ImGui::Button(GetDisplayName(gameobject.transform).data())) {
                        _active_component = &gameobject.transform;
                    }
                    ImGui::PopID();

                    // Other Components
                    for (const auto &component: gameobject.GetComponentsView()) {
                        ImGui::PushID(component->id);
                        ImGui::Bullet();
                        if (ImGui::Button(GetDisplayName(*component).data())) {
                            _active_component = component.get();
                        }
                        ImGui::PopID();
                    }

                    if (ImGui::SmallButton("Destroy")) {
                        _active_component = nullptr;
                        _scene.DestroyGameObject(gameobject);
                    }

                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        });
    }

    void EditorEngine::SetupInspectorUI() {
        _gui->AddItem("Inspector", [&]() {
            if (_active_component) {
                ImGui::SeparatorText(std::format("{0} :: {1}", GetDisplayName(_active_component->gameObject),
                                                 GetDisplayName(*_active_component))
                                             .data());
                auto instance = Reflect::GetType(typeid(*_active_component)).from_void(_active_component);
                DrawComponent(instance);
                if (ImGui::SmallButton("Remove")) {
                    _active_component->gameObject.RemoveComponent(*_active_component);
                    _active_component = nullptr;
                }
            }
        });
    }


    void EditorEngine::DrawComponent(entt::meta_any &component) {
        for (auto &&[id, base]: component.type().base()) {
            auto instance = base.from_void(component.data());
            DrawComponent(instance);
        }
        for (auto &&[id, elem]: component.type().data()) {
            auto instance = elem.get(component);
            DrawElement(Reflect::GetName(id), instance);
        }
    }


    void EditorEngine::DrawElement(const std::string_view name, entt::meta_any &element) {
        const auto &type = element.type();
        void *data = element.data();

        if (type == Reflect::GetType<bool>()) {
            ImGui::Checkbox(name.data(), static_cast<bool *>(data));
        } else if (type.is_integral()) {
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

    std::string EditorEngine::GetDisplayName(Object &object) {
        return std::format("{0} ({1})", object.name, object.id);
    }


} // namespace Vkxel
