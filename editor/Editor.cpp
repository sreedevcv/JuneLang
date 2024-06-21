#include "Editor.hpp"

#include <iostream>

#include <string>

jed::Editor::Editor()
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

    static const auto mouse_move_callback = [](GLFWwindow* glfw_window, double x_pos_in, double y_pos_in) {
    };

    static const auto mouse_scroll_callback = [](GLFWwindow* glfw_window, double x_offset, double y_offset) {
    };

    const auto framebuffer_size_callback = [](GLFWwindow* glfw_window, int width, int height) {
        glViewport(0, 0, width, height);
    };

    // glfwSetCursorPosCallback(m_window, mouse_move_callback);
    // glfwSetScrollCallback(m_window, mouse_scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

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
    m_shader.use();
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

    std::string text = std::string(R"(
class Cake [
    taste() [
        var adjective = "delicious";
        print "The " + self.flavor + " cake is " + adjective + "!";
    ]
]

var cake = Cake();
cake.flavor = "German chocolate";
cake.taste();

    )");

    m_data.m_data.push_back("class Cake [");
    m_data.m_data.push_back("    taste() [");
    m_data.m_data.push_back("        var adjective = \"delicious\";");
    m_data.m_data.push_back("    ]");
    m_data.m_data.push_back("]");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(m_width), 0.0f, static_cast<float>(m_height));
    check_for_opengl_error();

    while (!glfwWindowShouldClose(m_window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        handle_inputs();

        m_shader.set_uniform_matrix("projection", projection);
        // m_text_renderer.render_text(m_shader, text, 0.0f, 900.0f, 1.0f, glm::vec3(0.8f, 0.3f, 0.2f));
        m_text_renderer.render_text(m_shader, m_data, 0.0f, 880.0f, 1.0f, glm::vec3(0.8f, 0.3f, 0.2f));

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
