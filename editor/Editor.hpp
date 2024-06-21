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
    TextRender m_text_renderer;
    TextData m_data;
    Shader m_shader;

    int m_width = 1000;
    int m_height = 900;
    const char* title = "Editor";
    bool debug_mode;

    Cursor cursor = {
        .line = 10,
        .loc = 30,
    };

    void handle_inputs();
};
} // namespace jed
