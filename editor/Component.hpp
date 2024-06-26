#pragma once

#include "Rectangle.hpp"
#include "TextRender.hpp"
#include "Shader.hpp"
#include "Timer.hpp"

namespace jed {

class Component {
public:
    Component() = default;
    ~Component() = default;

    void load(int width, int height, int x, int y, glm::vec3 color);
    void set_data_source(TextData* data);
    void draw(float delta);

private:
    int m_width;
    int m_height;
    int m_x;
    int m_y;
    float m_scale;
    bool m_cursor_blink = false;
    glm::vec3 m_color;
    Shader m_shader;
    Rectangle m_rect;
    TextRender m_renderer;
    TextData* m_data;
    Cursor m_cursor = {0, 0};
    Timer m_cursor_timer = TImer(1.0f);
};

} // namespace jed
