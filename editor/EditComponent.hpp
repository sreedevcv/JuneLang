#pragma once

#include "Component.hpp"
#include "Timer.hpp"

namespace jed {

class EditComponent : public Component {
public:
    EditComponent() = default;
    virtual ~EditComponent() = default;

    void draw(float delta);

    /* Input Handling Functions */

    virtual void handle_text(char text) override;
    virtual void handle_enter() override;
    virtual void handle_arrow_left() override;
    virtual void handle_arrow_right() override;
    virtual void handle_arrow_up() override;
    virtual void handle_arrow_down() override;
    virtual void handle_backspace() override;
    virtual void handle_tab() override;
    virtual void handle_scroll_vert(float offset) override;
    virtual void handle_scroll_horz(float offset) override;

private:
    float m_scroll_speed = 10.0f;
    bool m_cursor_blink = false;
    Timer m_cursor_timer = Timer(1.0f);
};

} // namespace jed
