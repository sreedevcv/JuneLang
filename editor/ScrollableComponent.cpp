#include "ScrollableComponent.hpp"

void jed::ScrollableComponent::handle_scroll_vert(float offset)
{
    m_scroll_offset.y -= offset * m_scroll_speed;
}

void jed::ScrollableComponent::handle_scroll_horz(float offset)
{
    m_scroll_offset.x += offset * m_scroll_speed;
}
