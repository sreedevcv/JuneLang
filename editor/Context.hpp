#pragma once

#include "glm/ext.hpp"

namespace jed {
class Context {
public:
    int width = 1000.0f;
    int height = 900.0f;
    int tab_width = 4;

    float font_size = 24.0f;
    int mouse_x = 0;
    int mouse_y = 0;
    int data_grow_size = 20;
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

    static Context& get();

private:
    Context() = default;
    ~Context() = default;
};
} // namespace jed
