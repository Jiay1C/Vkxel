//
// Created by jiayi on 1/26/2025.
//

#include <unordered_map>
#include <unordered_set>

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

    glm::vec2 Input::GetMousePosition() { return _mousePosition; }

    glm::vec2 Input::GetMouseScrollDelta() { return _mouseScrollDelta; }

    GLFWwindow *Input::GetLastInputWindow() { return _last_input_window; }


    void Input::Update() {
        _keyDownSet.clear();
        _keyUpSet.clear();
        glfwPollEvents();
    }

    void Input::EnableKeyboardInput(const bool enable) { _enable_keyboard_input = enable; }

    void Input::EnableMouseInput(const bool enable) { _enable_mouse_input = enable; }


    void Input::glfwBindWindow(GLFWwindow *glfwWindow) {
        glfwSetKeyCallback(glfwWindow, glfwKeyCallback);
        glfwSetMouseButtonCallback(glfwWindow, glfwMouseButtonCallback);
        glfwSetCursorPosCallback(glfwWindow, glfwCursorPosCallback);
        glfwSetScrollCallback(glfwWindow, glfwScrollCallback);
    }

    void Input::glfwKeyCallback(GLFWwindow *glfwWindow, int glfwKey, [[maybe_unused]] int glfwScanCode, int glfwAction,
                                [[maybe_unused]] int glfwMods) {
        _last_input_window = glfwWindow;

        if (!_glfwKeyCodeMap.contains(glfwKey)) {
            return;
        }

        KeyCode keyCode = _glfwKeyCodeMap.at(glfwKey);

        if (glfwAction == GLFW_RELEASE) {
            _keyUpSet.emplace(keyCode);
            _keySet.erase(keyCode);
        } else if (glfwAction == GLFW_PRESS && _enable_keyboard_input) {
            _keyDownSet.emplace(keyCode);
            _keySet.emplace(keyCode);
        }
    }

    void Input::glfwMouseButtonCallback(GLFWwindow *glfwWindow, int glfwButton, int glfwAction,
                                        [[maybe_unused]] int glfwMods) {
        _last_input_window = glfwWindow;

        if (!_glfwKeyCodeMap.contains(glfwButton)) {
            return;
        }

        KeyCode keyCode = _glfwKeyCodeMap.at(glfwButton);

        if (glfwAction == GLFW_RELEASE) {
            _keyUpSet.emplace(keyCode);
            _keySet.erase(keyCode);
        } else if (glfwAction == GLFW_PRESS && _enable_mouse_input) {
            _keyDownSet.emplace(keyCode);
            _keySet.emplace(keyCode);
        }
    }

    void Input::glfwCursorPosCallback(GLFWwindow *glfwWindow, double glfwXPos, double glfwYPos) {
        _last_input_window = glfwWindow;

        int width, height;
        glfwGetWindowSize(glfwWindow, &width, &height);
        _mousePosition = {glfwXPos / height, glfwYPos / height};
    }


    void Input::glfwScrollCallback(GLFWwindow *glfwWindow, double glfwXOffset, double glfwYOffset) {
        _last_input_window = glfwWindow;

        if (_enable_keyboard_input) {
            _mouseScrollDelta = {glfwXOffset, glfwYOffset};
        }
    }

    const std::unordered_map<int, KeyCode> Input::_glfwKeyCodeMap = {
            {GLFW_MOUSE_BUTTON_LEFT, KeyCode::MOUSE_BUTTON_LEFT},
            {GLFW_MOUSE_BUTTON_RIGHT, KeyCode::MOUSE_BUTTON_RIGHT},
            {GLFW_MOUSE_BUTTON_MIDDLE, KeyCode::MOUSE_BUTTON_MIDDLE},
            {GLFW_KEY_0, KeyCode::KEY_0},
            {GLFW_KEY_1, KeyCode::KEY_1},
            {GLFW_KEY_2, KeyCode::KEY_2},
            {GLFW_KEY_3, KeyCode::KEY_3},
            {GLFW_KEY_4, KeyCode::KEY_4},
            {GLFW_KEY_5, KeyCode::KEY_5},
            {GLFW_KEY_6, KeyCode::KEY_6},
            {GLFW_KEY_7, KeyCode::KEY_7},
            {GLFW_KEY_8, KeyCode::KEY_8},
            {GLFW_KEY_9, KeyCode::KEY_9},
            {GLFW_KEY_A, KeyCode::KEY_A},
            {GLFW_KEY_B, KeyCode::KEY_B},
            {GLFW_KEY_C, KeyCode::KEY_C},
            {GLFW_KEY_D, KeyCode::KEY_D},
            {GLFW_KEY_E, KeyCode::KEY_E},
            {GLFW_KEY_F, KeyCode::KEY_F},
            {GLFW_KEY_G, KeyCode::KEY_G},
            {GLFW_KEY_H, KeyCode::KEY_H},
            {GLFW_KEY_I, KeyCode::KEY_I},
            {GLFW_KEY_J, KeyCode::KEY_J},
            {GLFW_KEY_K, KeyCode::KEY_K},
            {GLFW_KEY_L, KeyCode::KEY_L},
            {GLFW_KEY_M, KeyCode::KEY_M},
            {GLFW_KEY_N, KeyCode::KEY_N},
            {GLFW_KEY_O, KeyCode::KEY_O},
            {GLFW_KEY_P, KeyCode::KEY_P},
            {GLFW_KEY_Q, KeyCode::KEY_Q},
            {GLFW_KEY_R, KeyCode::KEY_R},
            {GLFW_KEY_S, KeyCode::KEY_S},
            {GLFW_KEY_T, KeyCode::KEY_T},
            {GLFW_KEY_U, KeyCode::KEY_U},
            {GLFW_KEY_V, KeyCode::KEY_V},
            {GLFW_KEY_W, KeyCode::KEY_W},
            {GLFW_KEY_X, KeyCode::KEY_X},
            {GLFW_KEY_Y, KeyCode::KEY_Y},
            {GLFW_KEY_Z, KeyCode::KEY_Z},
            {GLFW_KEY_ESCAPE, KeyCode::KEY_ESCAPE},
            {GLFW_KEY_SPACE, KeyCode::KEY_SPACE},
            {GLFW_KEY_LEFT_SHIFT, KeyCode::KEY_LEFT_SHIFT},
            {GLFW_KEY_ENTER, KeyCode::KEY_ENTER},
            {GLFW_KEY_TAB, KeyCode::KEY_TAB},
            {GLFW_KEY_BACKSPACE, KeyCode::KEY_BACKSPACE},
            {GLFW_KEY_INSERT, KeyCode::KEY_INSERT},
            {GLFW_KEY_DELETE, KeyCode::KEY_DELETE},
            {GLFW_KEY_RIGHT, KeyCode::KEY_RIGHT},
            {GLFW_KEY_LEFT, KeyCode::KEY_LEFT},
            {GLFW_KEY_DOWN, KeyCode::KEY_DOWN},
            {GLFW_KEY_UP, KeyCode::KEY_UP},
            {GLFW_KEY_PAGE_UP, KeyCode::KEY_PAGE_UP},
            {GLFW_KEY_PAGE_DOWN, KeyCode::KEY_PAGE_DOWN},
            {GLFW_KEY_HOME, KeyCode::KEY_HOME},
            {GLFW_KEY_END, KeyCode::KEY_END},
            {GLFW_KEY_CAPS_LOCK, KeyCode::KEY_CAPS_LOCK},
            {GLFW_KEY_SCROLL_LOCK, KeyCode::KEY_SCROLL_LOCK},
            {GLFW_KEY_NUM_LOCK, KeyCode::KEY_NUM_LOCK},
            {GLFW_KEY_PRINT_SCREEN, KeyCode::KEY_PRINT_SCREEN},
            {GLFW_KEY_PAUSE, KeyCode::KEY_PAUSE},
            {GLFW_KEY_F1, KeyCode::KEY_F1},
            {GLFW_KEY_F2, KeyCode::KEY_F2},
            {GLFW_KEY_F3, KeyCode::KEY_F3},
            {GLFW_KEY_F4, KeyCode::KEY_F4},
            {GLFW_KEY_F5, KeyCode::KEY_F5},
            {GLFW_KEY_F6, KeyCode::KEY_F6},
            {GLFW_KEY_F7, KeyCode::KEY_F7},
            {GLFW_KEY_F8, KeyCode::KEY_F8},
            {GLFW_KEY_F9, KeyCode::KEY_F9},
            {GLFW_KEY_F10, KeyCode::KEY_F10},
            {GLFW_KEY_F11, KeyCode::KEY_F11},
            {GLFW_KEY_F12, KeyCode::KEY_F12},
            {GLFW_KEY_KP_0, KeyCode::KEY_KP_0},
            {GLFW_KEY_KP_1, KeyCode::KEY_KP_1},
            {GLFW_KEY_KP_2, KeyCode::KEY_KP_2},
            {GLFW_KEY_KP_3, KeyCode::KEY_KP_3},
            {GLFW_KEY_KP_4, KeyCode::KEY_KP_4},
            {GLFW_KEY_KP_5, KeyCode::KEY_KP_5},
            {GLFW_KEY_KP_6, KeyCode::KEY_KP_6},
            {GLFW_KEY_KP_7, KeyCode::KEY_KP_7},
            {GLFW_KEY_KP_8, KeyCode::KEY_KP_8},
            {GLFW_KEY_KP_9, KeyCode::KEY_KP_9},
            {GLFW_KEY_KP_DECIMAL, KeyCode::KEY_KP_DECIMAL},
            {GLFW_KEY_KP_DIVIDE, KeyCode::KEY_KP_DIVIDE},
            {GLFW_KEY_KP_MULTIPLY, KeyCode::KEY_KP_MULTIPLY},
            {GLFW_KEY_KP_SUBTRACT, KeyCode::KEY_KP_SUBTRACT},
            {GLFW_KEY_KP_ADD, KeyCode::KEY_KP_ADD},
            {GLFW_KEY_KP_ENTER, KeyCode::KEY_KP_ENTER},
            {GLFW_KEY_KP_EQUAL, KeyCode::KEY_KP_EQUAL},
            {GLFW_KEY_LEFT_CONTROL, KeyCode::KEY_LEFT_CONTROL},
            {GLFW_KEY_LEFT_ALT, KeyCode::KEY_LEFT_ALT},
            {GLFW_KEY_LEFT_SUPER, KeyCode::KEY_LEFT_SUPER},
            {GLFW_KEY_RIGHT_SHIFT, KeyCode::KEY_RIGHT_SHIFT},
            {GLFW_KEY_RIGHT_CONTROL, KeyCode::KEY_RIGHT_CONTROL},
            {GLFW_KEY_RIGHT_ALT, KeyCode::KEY_RIGHT_ALT},
            {GLFW_KEY_RIGHT_SUPER, KeyCode::KEY_RIGHT_SUPER},
            {GLFW_KEY_MENU, KeyCode::KEY_MENU}};

    GLFWwindow *Input::_last_input_window = nullptr;
    bool Input::_enable_keyboard_input = true;
    bool Input::_enable_mouse_input = true;
    std::unordered_set<KeyCode> Input::_keySet;
    std::unordered_set<KeyCode> Input::_keyDownSet;
    std::unordered_set<KeyCode> Input::_keyUpSet;
    glm::vec2 Input::_mousePosition = {0.0f, 0.0f};
    glm::vec2 Input::_mouseScrollDelta = {0.0f, 0.0f};

} // namespace Vkxel
