#include "MainComponent.hpp"

#include "Context.hpp"

void jed::MainComponent::load_component()
{
    int main_x = m_gutter_width;
    int main_y = m_top_bar_height;
    int main_width = Context::get().width - main_x;
    int main_height = Context::get().height - main_y;

    set_bg_color(glm::vec3(0.8f, 0.9f, 0.76f));
    set_cursor_color(glm::vec3(0.3f, 0.3f, 0.3f));
    set_text_color(glm::vec3(0.5f, 0.6f, 0.4f));

    load(main_width, main_height, main_x, main_y);
    set_data_source(&m_text_data);

    int line_x = 0;
    int line_y = m_top_bar_height;
    int line_width = m_gutter_width;
    int line_height = Context::get().height - m_top_bar_height;
    m_line_gutter.load(line_width, line_height, line_x, line_y);
    m_line_gutter.set_data_source(&m_line_data);
    m_line_data.add_text_to_line('1', m_line_cursor);

    int bar_x = 0;
    int bar_y = 0;
    int bar_width = Context::get().width;
    int bar_height = m_top_bar_height;
    m_top_bar.load(bar_width, bar_height, bar_x, bar_y);
    m_top_bar.set_data_source(&m_top_bar_file_name);
    std::string file = "New File";
    set_current_file_name(file);

    int out_x = main_x;
    int out_y = Context::get().height * 0.7;
    int out_width = main_width;
    int out_height = Context::get().height - out_y;
    m_output_comp.load(out_width, out_height, out_x, out_y);
    m_output_comp.set_data_source(&m_output_data);
}

void jed::MainComponent::draw(float delta)
{
    EditComponent::draw(delta);
    m_line_gutter.draw(delta);
    m_top_bar.draw(delta);
    if (m_show_output_tab) {
        m_output_comp.draw(delta);
    }
}

void jed::MainComponent::set_new_data_source(TextData& data)
{
    Component::set_new_data_source(data);
    update_line_data();
}

void jed::MainComponent::handle_enter()
{
    EditComponent::handle_enter();
    update_line_data();
}

void jed::MainComponent::handle_backspace()
{
    EditComponent::handle_backspace();
    update_line_data();
}

void jed::MainComponent::handle_scroll_vert(float offset)
{
    if (!m_show_output_tab) {
        ScrollableComponent::handle_scroll_vert(offset);
        m_line_gutter.handle_scroll_vert(offset);
    } else {
        m_output_comp.handle_scroll_vert(offset);
    }
}

void jed::MainComponent::handle_scroll_horz(float offset)
{
    if (!m_show_output_tab) {
        ScrollableComponent::handle_scroll_horz(offset);
    } else {
        m_output_comp.handle_scroll_horz(offset);
    }
}

void jed::MainComponent::set_current_file_name(std::string& file_name)
{
    m_top_bar_file_name.clear();
    Cursor cursor = {0, 0};
    for(char c: file_name) {
        m_top_bar_file_name.add_text_to_line(c, cursor);
        cursor.loc += 1;
    }
}

void jed::MainComponent::delete_word()
{
    char curr = m_text_data.peek(m_cursor);
    while (curr != '\0' || curr != ' ') {
        EditComponent::handle_backspace();
        curr = m_text_data.peek(m_cursor);
    }
    update_line_data();
}

void jed::MainComponent::toggle_output_visibility()
{
    m_show_output_tab = !m_show_output_tab;
}

void jed::MainComponent::set_output_contents(std::string& data)
{
    m_output_data.clear();
    m_output_data.append_string(data);
    m_show_output_tab = true;
}

void jed::MainComponent::update_line_data()
{
    if (m_line_count == m_text_data.get_line_count()) {
        return;
    }

    m_line_data.clear();
    for (int i = 0; i < m_text_data.get_line_count(); i++) {
        std::string line = std::to_string(i + 1);
        m_line_cursor.loc = 0;
        m_line_cursor.line = i;

        for (char c : line) {
            m_line_data.add_text_to_line(c, m_line_cursor);
            m_line_cursor.loc += 1;
        }
    }

    m_line_count = m_text_data.get_line_count();
}
