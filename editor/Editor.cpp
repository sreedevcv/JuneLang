#include "Editor.hpp"

#include <iostream>

#include <string>

jed::Editor::Editor()
    : m_width(1000)
    , m_height(900)
    , m_text_renderer(TextRender(m_width, m_height))
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);

    if (m_window == nullptr) {
        std::cout << "Failed to load window\n";
        glfwTerminate();
        std::exit(-1);
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);

    static const auto mouse_move_callback = [](GLFWwindow* glfw_window, double x_pos_in, double y_pos_in) {
    };

    static const auto mouse_scroll_callback = [](GLFWwindow* glfw_window, double x_offset, double y_offset) {
    };

    const auto framebuffer_size_callback = [](GLFWwindow* glfw_window, int width, int height) {
        glViewport(0, 0, width, height);
    };

    const auto key_callback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ENTER) {
                editor->cursor.line += 1;
            }
        }
    };

    const auto charachter_callback = [](GLFWwindow* window, unsigned int codepoint) {
        Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(window));
        char text = static_cast<char>(codepoint);
        std::cout << text << " " << editor->cursor.loc << std::endl;
        // editor->m_data.add_text_to_line(text, editor->cursor);
        // editor->cursor.loc = editor->cursor.loc + 1;
    };

    // glfwSetCursorPosCallback(m_window, mouse_move_callback);
    // glfwSetScrollCallback(m_window, mouse_scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetCharCallback(m_window, charachter_callback);
    glfwSetKeyCallback(m_window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::exit(-1);
    }

    std::cout << "Window and OpenGL inititalized" << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_text_renderer.load_fonts();
    m_shader.create_shader_using_files("res/shaders/text.vert", "res/shaders/text.frag");
    m_shader.compile();
}

jed::Editor::~Editor()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << "Window destroyed\n";
}

void jed::Editor::start()
{
    glClearColor(0.85, 0.92, 1.0, 1.0);

    // m_data.m_data.push_back("class Cake [");
    // m_data.m_data.push_back("    taste() [");
    // m_data.m_data.push_back("        var adjective = \"delicious\";");
    // m_data.m_data.push_back("    ]");
    // m_data.m_data.push_back("]");

    check_for_opengl_error();

    while (!glfwWindowShouldClose(m_window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        handle_inputs();

        // m_text_renderer.render_text(m_shader, text, 0.0f, 900.0f, 1.0f, glm::vec3(0.8f, 0.3f, 0.2f));
        m_text_renderer.render_text(m_shader, m_data, 0.0f, 880.0f, 1.0f, glm::vec3(0.8f, 0.3f, 0.2f));
        m_text_renderer.render_cursor(m_shader, cursor);
        check_for_opengl_error();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    check_for_opengl_error();
}

void jed::Editor::handle_inputs()
{
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}
