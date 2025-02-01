//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_INPUT_H
#define VKXEL_INPUT_H

#include <unordered_map>
#include <unordered_set>

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "window.h"

namespace Vkxel {

    enum class KeyCode {
        MOUSE_BUTTON_LEFT,
        MOUSE_BUTTON_RIGHT,
        MOUSE_BUTTON_MIDDLE,
        KEY_0,
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,
        KEY_ESCAPE,
        KEY_SPACE,
        KEY_ENTER,
        KEY_TAB,
        KEY_BACKSPACE,
        KEY_INSERT,
        KEY_DELETE,
        KEY_RIGHT,
        KEY_LEFT,
        KEY_DOWN,
        KEY_UP,
        KEY_PAGE_UP,
        KEY_PAGE_DOWN,
        KEY_HOME,
        KEY_END,
        KEY_CAPS_LOCK,
        KEY_SCROLL_LOCK,
        KEY_NUM_LOCK,
        KEY_PRINT_SCREEN,
        KEY_PAUSE,
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_F11,
        KEY_F12,
        KEY_KP_0,
        KEY_KP_1,
        KEY_KP_2,
        KEY_KP_3,
        KEY_KP_4,
        KEY_KP_5,
        KEY_KP_6,
        KEY_KP_7,
        KEY_KP_8,
        KEY_KP_9,
        KEY_KP_DECIMAL,
        KEY_KP_DIVIDE,
        KEY_KP_MULTIPLY,
        KEY_KP_SUBTRACT,
        KEY_KP_ADD,
        KEY_KP_ENTER,
        KEY_KP_EQUAL,
        KEY_LEFT_CONTROL,
        KEY_LEFT_ALT,
        KEY_LEFT_SUPER,
        KEY_LEFT_SHIFT,
        KEY_RIGHT_SHIFT,
        KEY_RIGHT_CONTROL,
        KEY_RIGHT_ALT,
        KEY_RIGHT_SUPER,
        KEY_MENU
    };

    class Input {
    public:
        friend class Window;

        Input() = delete;
        ~Input() = delete;

        static bool GetKey(KeyCode key);
        static bool GetKeyDown(KeyCode key);
        static bool GetKeyUp(KeyCode key);

        static GLFWwindow *GetLastInputWindow();

        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseScrollDelta();

        static void EnableMouseInput(bool enable);
        static void EnableKeyboardInput(bool enable);

        static void Update();

    private:
        static void glfwBindWindow(GLFWwindow *glfwWindow);
        static void glfwKeyCallback(GLFWwindow *glfwWindow, int glfwKey, int glfwScanCode, int glfwAction,
                                    int glfwMods);
        static void glfwMouseButtonCallback(GLFWwindow *glfwWindow, int glfwButton, int glfwAction, int glfwMods);
        static void glfwScrollCallback(GLFWwindow *glfwWindow, double glfwXOffset, double glfwYOffset);
        static void glfwCursorPosCallback(GLFWwindow *glfwWindow, double glfwXPos, double glfwYPos);

        const static std::unordered_map<int, KeyCode> _glfwKeyCodeMap;

        static GLFWwindow *_last_input_window;
        static bool _enable_mouse_input;
        static bool _enable_keyboard_input;
        static std::unordered_set<KeyCode> _keySet;
        static std::unordered_set<KeyCode> _keyDownSet;
        static std::unordered_set<KeyCode> _keyUpSet;
        static glm::vec2 _mousePosition;
        static glm::vec2 _mouseScrollDelta;
    };

} // namespace Vkxel

#endif // VKXEL_INPUT_H
