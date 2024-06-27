#pragma once

#include "Component.hpp"

namespace jed {

class ScrollableComponent : public Component {
public:
    ScrollableComponent() = default;
    virtual ~ScrollableComponent() = default;

    virtual void handle_scroll_vert(float offset) override;
    virtual void handle_scroll_horz(float offset) override;

protected:
    float m_scroll_speed = 10.0f;
};

} // namespace jed
