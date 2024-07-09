#include "EditComponent.hpp"

#include "Context.hpp"

void jed::EditComponent::draw(float delta)
{
    Component::draw(delta);

    m_cursor_timer.update(delta);
    if (m_cursor_timer.finished()) {
        m_cursor_blink = !m_cursor_blink;
        m_cursor_timer.reset();
    }

    if (m_cursor_blink) {
        m_renderer.render_cursor(m_shader, m_cursor, m_scroll_offset, m_cursor_color);
    }
}

void jed::EditComponent::handle_text(char text)
{
    m_data->add_text_to_line(text, m_cursor);
    m_cursor.loc = m_cursor.loc + 1;
    m_cursor_blink = true;
}

void jed::EditComponent::handle_enter()
{
    m_data->handle_enter(m_cursor);
    m_cursor.line += 1;
    m_cursor.loc = 0;
    m_cursor_blink = true;
}

void jed::EditComponent::handle_arrow_left()
{
    if (m_cursor.loc > 0) {
        m_cursor.loc -= 1;
    }
    m_cursor_blink = true;
}

void jed::EditComponent::handle_arrow_right()
{
    m_cursor.loc += 1;
    if (m_cursor.loc > m_data->get_line_size(m_cursor.line)) {
        m_cursor.loc -= 1;
    }
    m_cursor_blink = true;
}

void jed::EditComponent::handle_arrow_up()
{
    if (m_cursor.line > 0) {
        m_cursor.line -= 1;
        m_data->limit_cursor_loc(m_cursor);
    }
    m_cursor_blink = true;
}

void jed::EditComponent::handle_arrow_down()
{
    m_cursor.line += 1;
    if (m_cursor.line >= m_data->get_line_count()) {
        m_cursor.line -= 1;
    }
    m_data->limit_cursor_loc(m_cursor);
    m_cursor_blink = true;
}

void jed::EditComponent::handle_backspace()
{
    m_data->handle_backspace(m_cursor);
    m_cursor_blink = true;
}

void jed::EditComponent::handle_tab()
{
    for (int i = 0; i < Context::get().tab_width; i++) {
        m_data->add_text_to_line(' ', m_cursor);
        m_cursor.loc += 1;
    }
    m_cursor_blink = true;
}

void jed::EditComponent::handle_mouse_click(MouseButton button)
{
    int x_dist = Context::get().mouse_x - m_scroll_offset.x - m_x;
    int loc_index = x_dist / m_font->m_cursor_advance;
    int y_dist = Context::get().mouse_y + m_scroll_offset.y - m_y;
    int line_index = y_dist / m_font->m_font_size;

    int line_count = m_data->get_line_count();

    /* Restrict line */
    if (line_index < line_count) {
        m_cursor.line = line_index;
    } else {
        m_cursor.line = line_count == 0 ? 0 : line_count - 1;
    }

    /* Restrict loc */
    int line_size = m_data->get_line_size(m_cursor.line);
    if (loc_index < line_size) {
        m_cursor.loc = loc_index;
    } else {
        m_cursor.loc = line_size == 0 ? 0 : line_size;
    }
    m_cursor_blink = true;
}
