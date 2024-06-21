#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "TextRender.hpp"

namespace jed {
class Editor {
public:
    Editor();
    ~Editor();

    void start();

private:
    GLFWwindow* m_window;
    TextData m_data;
    Shader m_shader;

    int m_width = 1000;
    int m_height = 900;
    TextRender m_text_renderer; // Should be below width and height declaration

    const char* title = "Editor";
    bool debug_mode;

    Cursor cursor = {
        .line = 0,
        .loc = 0,
    };

    void handle_inputs();
};
} // namespace jed
