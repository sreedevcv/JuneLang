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
}

void jed::MainComponent::draw(float delta)
{
    EditComponent::draw(delta);
    m_line_gutter.draw(delta);
}

void jed::MainComponent::set_new_data_source(TextData& data)
{
    m_text_data.clear();
    m_text_data = data;
    update_line_data();
}

void jed::MainComponent::handle_enter()
{
    update_line_data();
    EditComponent::handle_enter();
}

void jed::MainComponent::handle_backspace()
{
    update_line_data();
    EditComponent::handle_backspace();
}

void jed::MainComponent::update_line_data()
{
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
}
