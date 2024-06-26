#include "Component.hpp"

void jed::Component::load(int width, int height, int x, int y, glm::vec3 color)
{
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;
    m_color = color;

    m_rect = Rectangle(width, height, x, y, color);
    m_rect.load();
    m_renderer.load_fonts();
    m_shader.create_shader_using_files("res/shaders/text.vert", "res/shaders/text.frag");
    m_shader.compile();
}

void jed::Component::set_data_source(TextData* data)
{
    m_data = data;
}

void jed::Component::draw(float delta)
{
    m_cursor_timer.update(delta);
    m_renderer.render_text(m_shader, *m_data, m_x, m_y, m_scale, m_color);
    if (m_cursor_timer.finished()) {
        m_cursor_blink = !m_cursor_blink;
        m_cursor_timer.reset();
    }

    if (m_cursor_blink) {
        m_renderer.render_cursor(m_shader, m_cursor, delta);
    }
}