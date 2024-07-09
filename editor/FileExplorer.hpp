#pragma once

#include "DirectoryViewer.hpp"
#include "ScrollableComponent.hpp"

namespace jed {

class FileExplorer : public ScrollableComponent {
public:
    FileExplorer();
    ~FileExplorer() = default;

    void handle_enter() override;
    void handle_arrow_up() override;
    void handle_arrow_down() override;
    void handle_backspace() override;

    bool has_selected_file();
    std::string get_path();

private:
    int32_t m_curr_index = 0;
    DirectoryViewer m_viewer;
    TextData m_data;
    bool m_just_selected_file = false;

    void update_text_data();
    const char* file_name(const std::string& full_path) const;
};

} // namespace jed
