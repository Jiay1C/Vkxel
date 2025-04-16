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
            ImGui::Text(std::format("Window ({0}, {1})", _window->GetWidth(), _window->GetHeight()).data());
            ImGui::Text(std::format("Resolution ({0}, {1})", _window->GetFrameBufferWidth(),
                                    _window->GetFrameBufferHeight())
                                .data());
        });
    }

    void EditorEngine::SetupSceneUI() {
        _gui->AddItem("Scene", [&]() {
            for (auto &gameObject: _scene.GetGameObjectsView()) {
                if (!gameObject.transform.GetParent()) {
                    DrawGameObjectTree(gameObject);
                }
            }
        });
    }

    void EditorEngine::SetupInspectorUI() {
        _gui->AddItem("Inspector", [&]() {
            if (_active_gameobject) {
                DrawGameObject(*_active_gameobject);
            } else {
                ImGui::SeparatorText("");
            }
        });
    }

    void EditorEngine::DrawGameObjectTree(GameObject &gameObject) {
        ImGui::PushID(static_cast<int>(gameObject.id));

        const auto children = gameObject.transform.GetChildren();

        bool showTree = false;
        if (children.empty()) {
            ImGui::Bullet();
        } else {
            showTree = ImGui::TreeNode("");
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(GetDisplayName(gameObject).data())) {
            _active_gameobject = &gameObject;
        }

        if (showTree) {
            for (auto &child: children) {
                DrawGameObjectTree(child.get().gameObject);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void EditorEngine::DrawGameObject(GameObject &gameObject) {
        ImGui::SeparatorText(GetDisplayName(gameObject).data());

        ImGui::InputText(
                "Name", gameObject.name.data(), gameObject.name.capacity() + 1, ImGuiInputTextFlags_CallbackResize,
                [](ImGuiInputTextCallbackData *callback_data) {
                    if (callback_data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                        std::string &callback_str = *static_cast<std::string *>(callback_data->UserData);
                        callback_str.resize(callback_data->BufTextLen);
                        callback_data->Buf = callback_str.data();
                    }
                    return 0;
                },
                &gameObject.name);

        DrawComponent(gameObject.transform);

        for (const auto &component: gameObject.GetComponentsView()) {
            DrawComponent(*component);
        }

        ImGui::SeparatorText("");

        if (ImGui::SmallButton("Destroy")) {
            _active_gameobject = nullptr;
            _scene.DestroyGameObject(gameObject);
        }
    }

    void EditorEngine::DrawComponent(Component &component) {
        ImGui::PushID(static_cast<int>(component.id));
        ImGui::SeparatorText(GetDisplayName(component).data());
        auto instance = Reflect::GetType(typeid(component)).from_void(&component);
        DrawComponentInternal(instance);
        if (ImGui::SmallButton("Remove")) {
            component.gameObject.RemoveComponent(component);
        }
        ImGui::PopID();
    }

    void EditorEngine::DrawComponentInternal(entt::meta_any &component) {
        for (auto &&[id, base]: component.type().base()) {
            auto instance = base.from_void(component.data());
            DrawComponentInternal(instance);
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
