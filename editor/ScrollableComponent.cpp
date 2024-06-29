#include "ScrollableComponent.hpp"

void jed::ScrollableComponent::handle_scroll_vert(float offset)
{
    m_scroll_offset.y -= offset * m_scroll_speed;
    if (m_scroll_offset.y < 0) {
        m_scroll_offset.y = 0.0f;
    }
}

void jed::ScrollableComponent::handle_scroll_horz(float offset)
{
    m_scroll_offset.x += offset * m_scroll_speed;
    if (m_scroll_offset.x > 0.0f) {
        m_scroll_offset.x = 0.0f;
    }
}
