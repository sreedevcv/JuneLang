#pragma once

#include "EditComponent.hpp"
#include "FileExplorer.hpp"

namespace jed {

class MainComponent : public EditComponent {
public:
    MainComponent() = default;
    ~MainComponent() = default;

    void load_component();
    void draw(float delta);

    void handle_enter() override;
    void handle_backspace() override;
    void handle_arrow_up() override;
    void handle_arrow_down() override;
    void handle_scroll_vert(float offset) override;
    void handle_scroll_horz(float offset) override;

    void set_new_data_source(TextData& data) override;

    void set_current_file_name(std::string& file_name);
    void delete_word();
    void set_output_contents(std::string& data);

    void focus_output();
    void focus_file_explorer();

private:
    enum Comp {
        MAIN,
        OUTPUT,
        EXPLORER,
    };

    TextData m_text_data;
    TextData m_line_data;
    TextData m_top_bar_file_name;
    TextData m_output_data;
    Component m_top_bar;
    ScrollableComponent m_line_gutter;
    ScrollableComponent m_output_comp;
    Cursor m_line_cursor { 0, 0 };
    FileExplorer m_explorer;
    float m_gutter_width = 50.0f;
    float m_top_bar_height = 50.0f;
    int m_line_count = 1;
    Comp m_infocus;

    void update_line_data();
};

} // namespace jed
