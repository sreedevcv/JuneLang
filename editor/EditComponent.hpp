#pragma once

#include "Component.hpp"

namespace jed {

class EditComponent: public Component {
public:
    EditComponent() = default;
    ~EditComponent() = default;

    // void load(int width, int height, int x, int y, glm::vec3 color);
    // void set_data_source(TextData* data);
    void draw(float delta);

    /* Input Handling Functions */

    void handle_text(char text) override;
    void handle_enter() override;
    void handle_arrow_left() override;
    void handle_arrow_right() override;
    void handle_arrow_up() override;
    void handle_arrow_down() override;
    void handle_backspace() override;
    void handle_tab() override;
    void handle_scroll_vert(float offset) override;
    void handle_scroll_horz(float offset) override;

private:
};

} // namespace jed
