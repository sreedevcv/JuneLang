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

    void handle_text(char text);
    void handle_enter();
    void handle_arrow_left();
    void handle_arrow_right();
    void handle_arrow_up();
    void handle_arrow_down();
    void handle_backspace();
    void handle_tab();

private:
    int m_width;
    int m_height;
    int m_x;
    int m_y;
    float m_scale = 1.0f;
    bool m_cursor_blink = false;
    Shader m_shader;
    Rectangle m_rect;
    TextRender m_renderer = TextRender(100, 100, 100, 100);
    TextData* m_data;
    Cursor m_cursor = {0, 0};
    Timer m_cursor_timer = Timer(1.0f);

    glm::vec3 m_bg_color = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 m_cursor_color = glm::vec3(0.3, 0.3, 0.3);
    glm::vec3 m_text_color = glm::vec3(0.0, 0.2, 0.7);
};

} // namespace jed
