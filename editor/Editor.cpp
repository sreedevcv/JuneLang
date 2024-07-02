#include "Editor.hpp"

#include <iostream>
#include <string>

#include "Context.hpp"
#include "FileHandler.hpp"
#include "FontLoader.hpp"
#include "Rectangle.hpp"
#include "Timer.hpp"

#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

void jed::charachter_callback(GLFWwindow* window, unsigned int codepoint)
{
    jed::Editor* editor = static_cast<jed::Editor*>(glfwGetWindowUserPointer(window));
    char text = static_cast<char>(codepoint);
    editor->comp.handle_text(text);
};

jed::Editor::Editor()
    : m_width(1000)
    , m_height(900)
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
        Context::get().mouse_x = x_pos_in;
        Context::get().mouse_y = y_pos_in;
    };

    static const auto mouse_button_callback = [](GLFWwindow* glfw_window, int button, int action, int mods) {
        Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(glfw_window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            editor->comp.handle_mouse_click(Component::LEFT);
        }
    };

    static const auto mouse_scroll_callback = [](GLFWwindow* glfw_window, double x_offset, double y_offset) {
        Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(glfw_window));
        editor->comp.handle_scroll_vert(y_offset);
        editor->comp.handle_scroll_horz(x_offset);
    };

    const auto framebuffer_size_callback = [](GLFWwindow* glfw_window, int width, int height) {
        glViewport(0, 0, width, height);
        Context::get().width = width;
        Context::get().height = height;
    };

    const auto key_callback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(window));

        if (action == GLFW_PRESS) {
            if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_B) {
                MainComponent* mcomp = dynamic_cast<MainComponent*>(editor->in_focus);
                if (!mcomp) {
                    return;
                }
                auto code = mcomp->get_data();
                auto result = editor->run_code(code);
                mcomp->set_output_contents(result); // create a method for textData that is same as fh.open_and_read
                return;
            }
            if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_O) {
                MainComponent* mcomp = dynamic_cast<MainComponent*>(editor->in_focus);
                if (!mcomp) {
                    return;
                }

                FileHandler fh;
                std::string file = "examples/inc.jun";
                if (fh.open_and_read(file)) {
                    auto data = fh.get_text_data();
                    mcomp->set_new_data_source(fh.get_text_data());
                    mcomp->set_current_file_name(file);
                }
            }
            if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_APOSTROPHE) {
                MainComponent* mcomp = dynamic_cast<MainComponent*>(editor->in_focus);
                if (!mcomp) {
                    return;
                }

                mcomp->toggle_output_visibility();
            }
        }

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {

            if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_C) {
                std::cout << "Ctrl + C pressed" << std::endl;
                return;
            }
            if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_V) {
                std::cout << "Ctrl + V pressed" << std::endl;
                return;
            }
            if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_BACKSPACE) {
                std::cout << "Ctrl + V pressed" << std::endl;
                // editor->comp.delete_word();
                return;
            }

            switch (key) {
            case GLFW_KEY_ENTER:
                editor->in_focus->handle_enter();
                break;
            case GLFW_KEY_LEFT:
                editor->in_focus->handle_arrow_left();
                break;
            case GLFW_KEY_RIGHT:
                editor->in_focus->handle_arrow_right();
                break;
            case GLFW_KEY_UP:
                editor->in_focus->handle_arrow_up();
                break;
            case GLFW_KEY_DOWN:
                editor->in_focus->handle_arrow_down();
                break;
            case GLFW_KEY_BACKSPACE:
                editor->in_focus->handle_backspace();
                break;
            case GLFW_KEY_TAB:
                editor->in_focus->handle_tab();
                break;
            default:
                break;
            }
        }
    };

    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetCursorPosCallback(m_window, mouse_move_callback);
    glfwSetScrollCallback(m_window, mouse_scroll_callback);
    glfwSetCharCallback(m_window, charachter_callback);
    glfwSetKeyCallback(m_window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::exit(-1);
    }

    std::cout << "Window and OpenGL inititalized" << std::endl;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shader.create_shader_using_files("res/shaders/text.vert", "res/shaders/text.frag");
    m_shader.compile();
    FontLoader::load_fonts();
}

jed::Editor::~Editor()
{
    /* NOTE:: If the shader is destroyed after destroying glfw window it results in a segfault */
    glfwDestroyWindow(m_window);
    glfwTerminate();
    std::cout << "Window destroyed\n";
}

void jed::Editor::start()
{
    glfwSetWindowUserPointer(m_window, this);
    glClearColor(0.85, 0.92, 1.0, 1.0);
    check_for_opengl_error();

    float prev_time = glfwGetTime();
    glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f);
    comp.load_component();
    in_focus = &comp;

    scomp.load(200, 150, 300, 350);
    scomp.set_data_source(&m_data);
    // m_data.add_text_to_line('1', {0, 0});

    while (!glfwWindowShouldClose(m_window)) {
        float curr_time = glfwGetTime();
        float delta = curr_time - prev_time;
        prev_time = curr_time;

        glClear(GL_COLOR_BUFFER_BIT);
        handle_inputs(delta);

        comp.draw(delta);
        // scomp.draw(delta);

        check_for_opengl_error();
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    check_for_opengl_error();
}

void jed::Editor::handle_inputs(float delta)
{
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
}

std::string jed::Editor::run_code(std::string& code)
{
    jl::ErrorHandler::reset();
    jl::ErrorHandler::m_stream.setOutputToStr();
    std::string result = "";
    jl::Lexer lexer(code.c_str());
    std::string file_name = "LIVE";
    lexer.scan();

    if (jl::ErrorHandler::has_error()) {
        result = jl::ErrorHandler::m_stream.get_string_stream().str();
        return result;
    }

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    if (jl::ErrorHandler::has_error()) {
        result = jl::ErrorHandler::m_stream.get_string_stream().str();
        return result;
    }

    jl::Interpreter interpreter(file_name);

    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    if (jl::ErrorHandler::has_error()) {
        result = jl::ErrorHandler::m_stream.get_string_stream().str();
        return result;
    }

    jl::Value v;
    interpreter.interpret(stmts);

    result = jl::ErrorHandler::get_string_stream().str();

    /* Problem when plus_equal operator was added */

    // for (auto stmt : stmts) {
    //     delete stmt;
    // }

    return result;
};
