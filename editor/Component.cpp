#include "Component.hpp"

#include "Context.hpp"

void jed::Component::load(int width, int height, int x, int y, glm::vec3 color)
{
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;

    m_rect = Rectangle(width, height, x, Context::get().height - height - y, m_bg_color);
    m_rect.load();
    m_renderer = TextRender(m_width, m_height, x, y);
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

    m_rect.draw(m_shader, Context::get().projection);
    m_renderer.render_text(m_shader, *m_data, m_x, Context::get().height - m_y, m_scale, m_text_color);

    if (m_cursor_timer.finished()) {
        m_cursor_blink = !m_cursor_blink;
        m_cursor_timer.reset();
    }

    if (m_cursor_blink) {
        m_renderer.render_cursor(m_shader, m_cursor, m_cursor_color);
    }
}
void jed::Component::handle_text(char text)
{
    m_data->add_text_to_line(text, m_cursor);
    m_cursor.loc = m_cursor.loc + 1;
    m_cursor_blink = true;
}
void jed::Component::handle_enter()
{
    m_data->handle_enter(m_cursor);
    m_cursor.line += 1;
    m_cursor.loc = 0;
    m_cursor_blink = true;
}

void jed::Component::handle_arrow_left()
{
    if (m_cursor.loc > 0) {
        m_cursor.loc -= 1;
    }
    m_cursor_blink = true;
}

void jed::Component::handle_arrow_right()
{
    m_cursor.loc += 1;
    if (m_cursor.loc > m_data->get_line_size(m_cursor.line)) {
        m_cursor.loc -= 1;
    }
    m_cursor_blink = true;
}

void jed::Component::handle_arrow_up()
{
    if (m_cursor.line > 0) {
        m_cursor.line -= 1;
        m_data->bound_cursor_loc(m_cursor);
    }
    m_cursor_blink = true;
}

void jed::Component::handle_arrow_down()
{
    m_cursor.line += 1;
    if (m_cursor.line >= m_data->get_line_count()) {
        m_cursor.line -= 1;
    }
    m_data->bound_cursor_loc(m_cursor);
    m_cursor_blink = true;
}

void jed::Component::handle_backspace()
{
    m_data->handle_backspace(m_cursor);
    m_cursor_blink = true;
}

void jed::Component::handle_tab()
{
    for (int i = 0; i < Context::get().tab_width; i++) {
        m_data->add_text_to_line(' ', m_cursor);
        m_cursor.loc += 1;
    }
    m_cursor_blink = true;
}
