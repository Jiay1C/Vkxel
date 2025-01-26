//
// Created by jiayi on 1/26/2025.
//

#include <unordered_set>
#include <unordered_map>

#include "GLFW/glfw3.h"

#include "input.h"

namespace Vkxel {
    bool Input::GetKey(const KeyCode key) {
        if (_keySet.contains(key)) {
            return true;
        }
        return false;
    }

    bool Input::GetKeyDown(const KeyCode key) {
        if (_keyDownSet.contains(key)) {
            return true;
        }
        return false;
    }

    bool Input::GetKeyUp(const KeyCode key) {
        if (_keyUpSet.contains(key)) {
            return true;
        }
        return false;
    }

    glm::vec2 Input::GetMousePosition() {
        return _mousePosition;
    }

    glm::vec2 Input::GetMouseScrollDelta() {
        return _mouseScrollDelta;
    }

    void Input::Update() {
        _keyDownSet.clear();
        _keyUpSet.clear();
        glfwPollEvents();
    }

    void Input::glfwBindWindow(GLFWwindow *glfwWindow) {
        glfwSetKeyCallback(glfwWindow, glfwKeyCallback);
        glfwSetMouseButtonCallback(glfwWindow, glfwMouseButtonCallback);
        glfwSetCursorPosCallback(glfwWindow, glfwCursorPosCallback);
        glfwSetScrollCallback(glfwWindow, glfwScrollCallback);
    }

    void Input::glfwKeyCallback([[maybe_unused]] GLFWwindow *glfwWindow, int glfwKey, [[maybe_unused]] int glfwScanCode, int glfwAction, [[maybe_unused]] int glfwMods) {
        if (!_glfwKeyCodeMap.contains(glfwKey)) {
            return;
        }

        KeyCode keyCode = _glfwKeyCodeMap.at(glfwKey);

        if (glfwAction == GLFW_RELEASE) {
            _keyUpSet.emplace(keyCode);
            _keySet.erase(keyCode);
        }
        else if (glfwAction == GLFW_PRESS) {
            _keyDownSet.emplace(keyCode);
            _keySet.emplace(keyCode);
        }
    }

    void Input::glfwMouseButtonCallback([[maybe_unused]] GLFWwindow *glfwWindow, int glfwButton, int glfwAction, [[maybe_unused]] int glfwMods) {
        if (!_glfwKeyCodeMap.contains(glfwButton)) {
            return;
        }

        KeyCode keyCode = _glfwKeyCodeMap.at(glfwButton);

        if (glfwAction == GLFW_RELEASE) {
            _keyUpSet.emplace(keyCode);
            _keySet.erase(keyCode);
        }
        else if (glfwAction == GLFW_PRESS) {
            _keyDownSet.emplace(keyCode);
            _keySet.emplace(keyCode);
        }
    }

    void Input::glfwCursorPosCallback([[maybe_unused]] GLFWwindow *glfwWindow, double glfwXPos, double glfwYPos) {
        _mousePosition = {glfwXPos, glfwYPos};
    }


    void Input::glfwScrollCallback([[maybe_unused]] GLFWwindow *glfwWindow, double glfwXOffset, double glfwYOffset) {
        _mouseScrollDelta = {glfwXOffset, glfwYOffset};
    }

    const std::unordered_map<int, KeyCode> Input::_glfwKeyCodeMap = {
        {GLFW_MOUSE_BUTTON_LEFT,  KeyCode::MOUSE_BUTTON_LEFT,  },
        {GLFW_MOUSE_BUTTON_RIGHT, KeyCode::MOUSE_BUTTON_RIGHT, },
        {GLFW_MOUSE_BUTTON_MIDDLE,KeyCode::MOUSE_BUTTON_MIDDLE,},
        {GLFW_KEY_0,              KeyCode::KEY_0,              },
        {GLFW_KEY_1,              KeyCode::KEY_1,              },
        {GLFW_KEY_2,              KeyCode::KEY_2,              },
        {GLFW_KEY_3,              KeyCode::KEY_3,              },
        {GLFW_KEY_4,              KeyCode::KEY_4,              },
        {GLFW_KEY_5,              KeyCode::KEY_5,              },
        {GLFW_KEY_6,              KeyCode::KEY_6,              },
        {GLFW_KEY_7,              KeyCode::KEY_7,              },
        {GLFW_KEY_8,              KeyCode::KEY_8,              },
        {GLFW_KEY_9,              KeyCode::KEY_9,              },
        {GLFW_KEY_A,              KeyCode::KEY_A,              },
        {GLFW_KEY_B,              KeyCode::KEY_B,              },
        {GLFW_KEY_C,              KeyCode::KEY_C,              },
        {GLFW_KEY_D,              KeyCode::KEY_D,              },
        {GLFW_KEY_E,              KeyCode::KEY_E,              },
        {GLFW_KEY_F,              KeyCode::KEY_F,              },
        {GLFW_KEY_G,              KeyCode::KEY_G,              },
        {GLFW_KEY_H,              KeyCode::KEY_H,              },
        {GLFW_KEY_I,              KeyCode::KEY_I,              },
        {GLFW_KEY_J,              KeyCode::KEY_J,              },
        {GLFW_KEY_K,              KeyCode::KEY_K,              },
        {GLFW_KEY_L,              KeyCode::KEY_L,              },
        {GLFW_KEY_M,              KeyCode::KEY_M,              },
        {GLFW_KEY_N,              KeyCode::KEY_N,              },
        {GLFW_KEY_O,              KeyCode::KEY_O,              },
        {GLFW_KEY_P,              KeyCode::KEY_P,              },
        {GLFW_KEY_Q,              KeyCode::KEY_Q,              },
        {GLFW_KEY_R,              KeyCode::KEY_R,              },
        {GLFW_KEY_S,              KeyCode::KEY_S,              },
        {GLFW_KEY_T,              KeyCode::KEY_T,              },
        {GLFW_KEY_U,              KeyCode::KEY_U,              },
        {GLFW_KEY_V,              KeyCode::KEY_V,              },
        {GLFW_KEY_W,              KeyCode::KEY_W,              },
        {GLFW_KEY_X,              KeyCode::KEY_X,              },
        {GLFW_KEY_Y,              KeyCode::KEY_Y,              },
        {GLFW_KEY_Z,              KeyCode::KEY_Z,              },
        {GLFW_KEY_ESCAPE,         KeyCode::KEY_ESCAPE,         },
        {GLFW_KEY_SPACE,          KeyCode::KEY_SPACE,          },
    };

    std::unordered_set<KeyCode> Input::_keySet;
    std::unordered_set<KeyCode> Input::_keyDownSet;
    std::unordered_set<KeyCode> Input::_keyUpSet;
    glm::vec2 Input::_mousePosition = {0.0f, 0.0f};
    glm::vec2 Input::_mouseScrollDelta = {0.0f, 0.0f};

} // Vkxel