#include "RotateFilter.h"

RotateFilter::RotateFilter(int angle)
    : m_angle {angle} {}

int RotateFilter::getAngle() const {
    return m_angle;
}

void RotateFilter::setAngle(int angle) {

    if (m_angle != angle) {
        m_angle = angle;
    }
}

bool RotateFilter::isActive() const {
    return m_angle % 360 != 0;
}

QImage RotateFilter::apply(const QImage& input) const {

    if (!isActive()) return input;

    QTransform t{};

    t.rotate(m_angle);

    QImage result { input.transformed(t) };


    return result;
}



