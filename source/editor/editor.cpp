//
// Created by jiayi on 2/21/2025.
//

#include <cstdint>
#include <format>
#include <vector>

#include "editor.h"

#include "custom/dual_contouring.h"
#include "custom/gpu_dual_contouring.h"
#include "engine/engine.h"
#include "engine/vtime.h"
#include "reflect/reflect.hpp"
#include "world/canvas.h"
#include "world/controller.h"
#include "world/drawer.h"
#include "world/mesh.h"
#include "world/mover.h"

namespace Vkxel {

    namespace {

        template<typename Type>
        bool DrawIntegral(const std::string_view name, entt::meta_any &element, const entt::meta_type &type,
                          const ImGuiDataType data_type) {
            if (type != Reflect::GetType<Type>()) {
                return false;
            }

            if (auto *value = element.try_cast<Type>(); value) {
                ImGui::DragScalar(name.data(), data_type, value);
            }

            return true;
        }

    } // namespace

    EditorEngine::EditorEngine(Scene &scene) : Engine(scene) {
        InitializeComponents();
        SetupDebugUI();
        SetupSceneUI();
        SetupInspectorUI();
    }

    void EditorEngine::InitializeComponents() {
        auto populateComponentList = [&](const auto& self, const Reflect::Type &type) -> void {
            for (const auto &derived: Reflect::GetDerived(type)) {
                if (derived != Reflect::GetType<Transform>()) {
                    _available_components.push_back(derived);
                }
                self(self, derived);
            }
        };

        populateComponentList(populateComponentList, Reflect::GetType<Component>());

        std::ranges::sort(_available_components, [](const Reflect::Type &lhs, const Reflect::Type &rhs) {
            return lhs.info().name() < rhs.info().name();
        });
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
            ImGui::Separator();
            DrawCreateGameObject();
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

        bool show_tree = false;
        if (children.empty()) {
            ImGui::Bullet();
            ImGui::SameLine();
            ImGui::Text("");
        } else {
            show_tree = ImGui::TreeNode("");
        }

        ImGui::SameLine();
        if (ImGui::SmallButton(GetDisplayName(gameObject).data())) {
            _active_gameobject = &gameObject;
        }

        ImGui::SameLine();
        DrawCreateGameObject(gameObject.transform);
        ImGui::SameLine();
        if (ImGui::SmallButton("-")) {
            _active_gameobject = nullptr;
            _scene.DestroyGameObject(gameObject);
        }

        if (show_tree) {
            for (auto &child: children) {
                DrawGameObjectTree(child.get().gameObject);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void EditorEngine::DrawGameObject(GameObject &gameObject) {
        ImGui::SeparatorText(GetDisplayName(gameObject).data());

        DrawString("Name", gameObject.name);

        DrawComponent(gameObject.transform);

        for (const auto &component: gameObject.GetComponentsView()) {
            DrawComponent(*component);
        }

        ImGui::SeparatorText("");
        DrawCreateComponent(gameObject);
        ImGui::SeparatorText("");
    }

    void EditorEngine::DrawComponent(Component &component) {
        ImGui::PushID(static_cast<int>(component.id));
        ImGui::SeparatorText(GetDisplayName(component).data());
        ImGui::SameLine();
        if (ImGui::SmallButton("-")) {
            component.gameObject.RemoveComponent(component);
        }
        auto instance = Reflect::GetType(typeid(component)).from_void(&component);

        const auto draw_type = [&](const auto &self, const entt::meta_type &type) -> void {
            for (auto &&[id, base]: type.base()) {
                self(self, base);
            }
            for (auto &&[id, elem]: type.data()) {
                if (auto element = elem.get(instance); element) {
                    DrawElement(Reflect::GetName(id), element);
                }
            }
        };

        draw_type(draw_type, instance.type());
        ImGui::PopID();
    }

    void EditorEngine::DrawElement(const std::string_view name, entt::meta_any &element) {
        if (!element) {
            ImGui::Text("%s: Unavailable", name.data());
            return;
        }

        const auto &type = element.type();

        if (type == Reflect::GetType<bool>()) {
            if (auto *value = element.try_cast<bool>(); value) {
                ImGui::Checkbox(name.data(), value);
            }
        } else if (DrawIntegral<std::int8_t>(name, element, type, ImGuiDataType_S8) ||
                   DrawIntegral<std::uint8_t>(name, element, type, ImGuiDataType_U8) ||
                   DrawIntegral<std::int16_t>(name, element, type, ImGuiDataType_S16) ||
                   DrawIntegral<std::uint16_t>(name, element, type, ImGuiDataType_U16) ||
                   DrawIntegral<std::int32_t>(name, element, type, ImGuiDataType_S32) ||
                   DrawIntegral<std::uint32_t>(name, element, type, ImGuiDataType_U32) ||
                   DrawIntegral<std::int64_t>(name, element, type, ImGuiDataType_S64) ||
                   DrawIntegral<std::uint64_t>(name, element, type, ImGuiDataType_U64)) {
        } else if (type == Reflect::GetType<float>()) {
            if (auto *value = element.try_cast<float>(); value) {
                ImGui::DragFloat(name.data(), value);
            }
        } else if (type == Reflect::GetType<double>()) {
            if (auto *value = element.try_cast<double>(); value) {
                ImGui::InputDouble(name.data(), value);
            }
        } else if (type == Reflect::GetType<glm::vec2>()) {
            if (auto *value = element.try_cast<glm::vec2>(); value) {
                ImGui::DragFloat2(name.data(), reinterpret_cast<float *>(value));
            }
        } else if (type == Reflect::GetType<glm::vec3>()) {
            if (auto *value = element.try_cast<glm::vec3>(); value) {
                ImGui::DragFloat3(name.data(), reinterpret_cast<float *>(value));
            }
        } else if (type == Reflect::GetType<glm::vec4>()) {
            if (auto *value = element.try_cast<glm::vec4>(); value) {
                ImGui::DragFloat4(name.data(), reinterpret_cast<float *>(value));
            }
        } else if (type == Reflect::GetType<glm::ivec2>()) {
            if (auto *value = element.try_cast<glm::ivec2>(); value) {
                ImGui::DragInt2(name.data(), reinterpret_cast<int *>(value));
            }
        } else if (type == Reflect::GetType<glm::ivec3>()) {
            if (auto *value = element.try_cast<glm::ivec3>(); value) {
                ImGui::DragInt3(name.data(), reinterpret_cast<int *>(value));
            }
        } else if (type == Reflect::GetType<glm::ivec4>()) {
            if (auto *value = element.try_cast<glm::ivec4>(); value) {
                ImGui::DragInt4(name.data(), reinterpret_cast<int *>(value));
            }
        } else if (type == Reflect::GetType<glm::quat>()) {
            if (auto *quat = element.try_cast<glm::quat>(); quat) {
                auto deg = glm::degrees(glm::eulerAngles(*quat));
                ImGui::DragFloat3(name.data(), reinterpret_cast<float *>(&deg));
                *quat = glm::radians(deg);
            }
        } else if (type == Reflect::GetType<std::string>()) {
            if (auto *str = element.try_cast<std::string>(); str) {
                DrawString(name, *str);
            }
        } else if (type.is_enum()) {
            std::vector<const char *> enum_names;
            std::vector<entt::meta_any> enum_values;
            int selected_enum = 0;
            int current_index = 0;
            for (auto &&[id, elem]: type.data()) {
                enum_names.push_back(Reflect::GetName(id).data());
                enum_values.push_back(elem.get(entt::meta_handle{}));
                if (enum_values.back() == element) {
                    selected_enum = current_index;
                }
                ++current_index;
            }
            if (!enum_names.empty() &&
                ImGui::Combo(name.data(), &selected_enum, enum_names.data(), static_cast<int>(enum_names.size()))) {
                element.assign(enum_values[static_cast<size_t>(selected_enum)]);
            }
        } else {
            ImGui::Text("%s: Unsupported Type <%s>", name.data(), type.info().name().data());
        }
    }

    void EditorEngine::DrawCreateGameObject(std::optional<std::reference_wrapper<Transform>> parent) {
        if (ImGui::SmallButton("+")) {
            GameObject &game_object = _scene.CreateGameObject();
            game_object.name = Application::DefaultGameObjectName;
            game_object.transform.SetParent(parent);
        }
    }

    void EditorEngine::DrawCreateComponent(GameObject &gameObject) {
        std::vector<Reflect::Type> available_types;

        for (const auto &type: _available_components) {
            if (!gameObject.GetComponent(type)) {
                available_types.push_back(type);
            }
        }

        std::vector<const char *> component_names;
        component_names.reserve(available_types.size());
        for (const auto &type: available_types) {
            component_names.push_back(type.info().name().data());
        }

        if (available_types.empty()) {
            const char *empty_label = "No Components";
            int empty_index = 0;
            ImGui::BeginDisabled();
            ImGui::Combo("##Component", &empty_index, &empty_label, 1);
            ImGui::SameLine();
            ImGui::Button("+");
            ImGui::EndDisabled();
            _selected_component = -1;
            return;
        }

        _selected_component = std::clamp(_selected_component, 0, static_cast<int>(available_types.size()) - 1);

        ImGui::Combo("##Component", &_selected_component, component_names.data(),
                     static_cast<int>(component_names.size()));

        ImGui::SameLine();

        if (ImGui::Button("+")) {
            gameObject.AddComponent(available_types[static_cast<size_t>(_selected_component)]);
        }
    }

    void EditorEngine::DrawString(const std::string_view name, std::string &str) {
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
    }


    std::string EditorEngine::GetDisplayName(Object &object) {
        return std::format("{0} ({1})", object.name, object.id);
    }


} // namespace Vkxel
