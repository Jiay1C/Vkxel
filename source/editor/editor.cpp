//
// Created by jiayi on 2/21/2025.
//

#include <format>

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
        DrawComponentInternal(instance);
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
            DrawString(name, str);
        } else if (type.is_enum()) {
            std::vector<const char *> enum_names;
            for (auto &&[id, elem]: type.data()) {
                enum_names.push_back(Reflect::GetName(id).data());
            }
            // TODO: Assume Use Int To Store Enum Here, Maybe Cause Bug
            ImGui::Combo(name.data(), static_cast<int *>(data), enum_names.data(), static_cast<int>(enum_names.size()));
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
        // Hard To Use Reflection, Hideous Hard Code Here
        // TODO: Use Reflection

        constexpr std::array component_names = {"Mover",  "Mesh",       "Drawer",         "Controller",       "Camera",
                                                "Canvas", "SDFSurface", "DualContouring", "GpuDualContouring"};
        std::array component_lambda = {std::function([&]() { gameObject.AddComponent<Mover>(); }),
                                       std::function([&]() { gameObject.AddComponent<Mesh>(); }),
                                       std::function([&]() { gameObject.AddComponent<Drawer>(); }),
                                       std::function([&]() { gameObject.AddComponent<Controller>(); }),
                                       std::function([&]() { gameObject.AddComponent<Camera>(); }),
                                       std::function([&]() { gameObject.AddComponent<Canvas>(); }),
                                       std::function([&]() { gameObject.AddComponent<SDFSurface>(); }),
                                       std::function([&]() { gameObject.AddComponent<DualContouring>(); }),
                                       std::function([&]() { gameObject.AddComponent<GpuDualContouring>(); })};


        ImGui::Combo("##Component", &_selected_component, component_names.data(),
                     static_cast<int>(component_names.size()));

        ImGui::SameLine();

        if (ImGui::Button("+")) {
            component_lambda[_selected_component]();
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
