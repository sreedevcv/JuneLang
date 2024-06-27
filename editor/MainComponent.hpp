#pragma once

#include "EditComponent.hpp"

namespace jed {

class MainComponent: public EditComponent {
public:
    MainComponent() = default;
    ~MainComponent() = default;

    void load_component();
    void draw(float delta);

    void set_new_data_source(TextData& data);

    void handle_enter() override;
    void handle_backspace() override;

private:
    TextData m_text_data;
    TextData m_line_data;
    Component m_line_gutter;
    Component m_top_bar;
    float m_gutter_width = 50.0f;
    float m_top_bar_height = 50.0f;

    Cursor m_line_cursor{0, 0};
    int m_line_count = 1;

    void update_line_data();
};

} // namespace jed
