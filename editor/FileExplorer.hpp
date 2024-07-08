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

private:
    uint32_t m_curr_index = 0;
    DirectoryViewer m_viewer;
    TextData m_data;

    void update_text_data();
};

} // namespace jed
