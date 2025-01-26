//
// Created by jiayi on 1/26/2025.
//

#ifndef VKXEL_INPUT_H
#define VKXEL_INPUT_H

#include <unordered_set>
#include <unordered_map>

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
    };

    class Input {
    public:
        friend class Window;
        Input()=delete;
        ~Input()=delete;
        static bool GetKey(KeyCode key);
        static bool GetKeyDown(KeyCode key);
        static bool GetKeyUp(KeyCode key);
        static glm::vec2 GetMousePosition();
        static glm::vec2 GetMouseScrollDelta();

        static void Update();

    private:
        static void glfwBindWindow(GLFWwindow *glfwWindow);
        static void glfwKeyCallback(GLFWwindow *glfwWindow, int glfwKey, int glfwScanCode, int glfwAction, int glfwMods);
        static void glfwMouseButtonCallback(GLFWwindow* glfwWindow, int glfwButton, int glfwAction, int glfwMods);
        static void glfwScrollCallback(GLFWwindow* glfwWindow, double glfwXOffset, double glfwYOffset);
        static void glfwCursorPosCallback(GLFWwindow* glfwWindow, double glfwXPos, double glfwYPos);

        const static std::unordered_map<int, KeyCode> _glfwKeyCodeMap;

        static std::unordered_set<KeyCode> _keySet;
        static std::unordered_set<KeyCode> _keyDownSet;
        static std::unordered_set<KeyCode> _keyUpSet;
        static glm::vec2 _mousePosition;
        static glm::vec2 _mouseScrollDelta;
    };

} // Vkxel

#endif //VKXEL_INPUT_H
