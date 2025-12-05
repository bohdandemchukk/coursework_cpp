#include "BWFilter.h"


BWFilter::BWFilter(bool enabled)
    : m_enabled {enabled} {}

bool BWFilter::isActive() const  {
    return getEnabled();
}

bool BWFilter::getEnabled() const {
    return m_enabled;
}

void BWFilter::setEnabled() {
    m_enabled = !m_enabled;
}

QImage BWFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};
    int height {result.height()};
    int width {result.width()};

    for (int y {0}; y < height; y++) {
        QRgb *row = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x {0}; x < width; x++) {
            int gray {qGray(row[x])};
            row[x] = qRgb(gray, gray, gray);
        }
    }

    return result;
}
