#include "FlipFilter.h"

FlipFilter::FlipFilter(Direction direction, bool enabled)
    : m_direction{direction}, m_enabled{enabled} {}



bool FlipFilter::isActive() const {
    return getEnabled();
}

FlipFilter::Direction FlipFilter::getDirection() const {
    return m_direction;
}

void FlipFilter::setDirection(Direction direction) {
    m_direction = direction;
}

bool FlipFilter::getEnabled() const {
    return m_enabled;
}

void FlipFilter::setEnabled() {
    m_enabled = !m_enabled;
}

QImage FlipFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    return input.mirrored(m_direction == Direction::Horizontal, m_direction == Direction::Vertical);
}


std::unique_ptr<ImageFilter> FlipFilter::clone() const {
    return std::make_unique<FlipFilter>(*this);
}
