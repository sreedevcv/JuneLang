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
    friend void charachter_callback(GLFWwindow* window, unsigned int codepoint);

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

    std::string testString = "";

    void handle_inputs(float delta);

};
} // namespace jed
