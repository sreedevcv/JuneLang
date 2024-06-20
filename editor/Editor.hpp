#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace jed {
class Editor {
public:
    Editor();
    ~Editor();

    void start();

private:
    GLFWwindow* m_window;

    int m_width = 800;
    int m_height = 600;
    const char* title = "Editor";
    bool debug_mode;
};
} // namespace jed
