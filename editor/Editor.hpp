#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "TextRender.hpp"
#include "MainComponent.hpp"

namespace jed {
class Editor {
public:
    Editor();
    ~Editor();

    void start();
    friend void charachter_callback(GLFWwindow* window, unsigned int codepoint);
    MainComponent comp;

private:
    GLFWwindow* m_window;
    TextData m_data;
    Shader m_shader;
    int m_width = 1000;
    int m_height = 900;
    const char* title = "Editor";
    bool debug_mode;
    bool m_cursor_blink = true;

    void handle_inputs(float delta);
    std::string run_code(std::string& code);
};

void charachter_callback(GLFWwindow* window, unsigned int codepoint);
} // namespace jed
