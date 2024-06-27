#pragma once

#include "Rectangle.hpp"
#include "TextRender.hpp"
#include "Shader.hpp"

namespace jed {

class Component {
public:
    Component() = default;
    virtual ~Component() = default;

    void load(int width, int height, int x, int y);
    void set_data_source(TextData* data);
    virtual void draw(float delta);

    /* Input Handling Functions */

    virtual void handle_text(char text);
    virtual void handle_enter();
    virtual void handle_arrow_left();
    virtual void handle_arrow_right();
    virtual void handle_arrow_up();
    virtual void handle_arrow_down();
    virtual void handle_backspace();
    virtual void handle_tab();
    virtual void handle_scroll_vert(float offset);
    virtual void handle_scroll_horz(float offset);

    void set_cursor_color(glm::vec3&& color);
    void set_bg_color(glm::vec3&& color);
    void set_text_color(glm::vec3&& color);

protected:
    int m_width;
    int m_height;
    int m_x;
    int m_y;
    float m_scale = 1.0f;

    Shader m_shader;
    Rectangle m_rect;
    TextRender m_renderer = TextRender(100, 100, 100, 100);
    TextData* m_data;
    Cursor m_cursor = {0, 0};

    glm::vec3 m_bg_color = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 m_cursor_color = glm::vec3(0.3, 0.3, 0.3);
    glm::vec3 m_text_color = glm::vec3(0.0, 0.2, 0.7);
    glm::vec2 m_scroll_offset = {0.0f, 0.0f};
};

} // namespace jed
