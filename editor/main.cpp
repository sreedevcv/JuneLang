#include <print>

#include "Editor.hpp"

int main()
{
    std::println("Hello World");

    {
        auto editor = jed::Editor();
        editor.start();
    }

    glfwTerminate();

    return 0;
}