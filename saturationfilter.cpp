#include "saturationfilter.h"

SaturationFilter::SaturationFilter(int saturation)
    : m_saturation{saturation} {}

bool SaturationFilter::isActive() const {
    return m_saturation != 0;
}

int SaturationFilter::getSaturation() const {
    return m_saturation;
}

void SaturationFilter::setSaturation(int saturation) {
    m_saturation = saturation;
}

QImage SaturationFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    double factor = static_cast<double>(getSaturation()) / 100.0 + 1.0;
    QImage result {input.copy()};
    int height {result.height()};
    int width {result.width()};

    for (int y {0}; y < height; ++y) {
        QRgb *row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x {0}; x < width; ++x) {
            QColor color {QColor::fromRgb(row[x])};

            int h, s, l;
            color.getHsl(&h, &s, &l);
            s = std::clamp(static_cast<int>(s * factor), 0, 255);
            color.setHsl(h, s, l);

            row[x] = color.rgb();
        }
    }

    return result;
}









