#include "Component.hpp"

#include "Context.hpp"

void jed::Component::load(int width, int height, int x, int y)
{
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;

    m_rect = Rectangle(width, height, x, Context::get().height - height - y, m_bg_color);
    m_rect.load();
    m_renderer = TextRender(m_width, m_height, x, m_y);
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
    m_rect.draw(m_shader, Context::get().projection);
    m_renderer.render_text(m_shader, *m_data, m_scroll_offset, m_scale, m_text_color);
}

void jed::Component::handle_text(char text)
{
}

void jed::Component::handle_enter()
{
}

void jed::Component::handle_arrow_left()
{
}

void jed::Component::handle_arrow_right()
{
}

void jed::Component::handle_arrow_up()
{
}

void jed::Component::handle_arrow_down()
{
}

void jed::Component::handle_backspace()
{
}

void jed::Component::handle_tab()
{
}

void jed::Component::handle_scroll_vert(float offset)
{
}

void jed::Component::handle_scroll_horz(float offset)
{
}

void jed::Component::set_cursor_color(glm::vec3&& color)
{
    m_cursor_color = color;
}

void jed::Component::set_bg_color(glm::vec3&& color)
{
    m_bg_color = color;
}

void jed::Component::set_text_color(glm::vec3&& color)
{
    m_text_color = color;
}