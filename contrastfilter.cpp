#include "contrastfilter.h"


ContrastFilter::ContrastFilter(int contrast)
    : m_contrast{contrast} {}

bool ContrastFilter::isActive() const {
    return m_contrast != 0;
}

int ContrastFilter::getContrast() const {
    return m_contrast;
}

void ContrastFilter::setContrast(int contrast) {
    m_contrast = contrast;
}

QImage ContrastFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    int contrast {getContrast()};

    double factor {(259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast))};
    QImage result {input.copy()};
    int height {result.height()};
    int width {result.width()};

    for (int y {0}; y < height; ++y) {
        QRgb *row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x {0}; x < width; ++x) {

            QRgb px {row[x]};

            int r {qRed(px)};
            int g {qGreen(px)};
            int b {qBlue(px)};

            r = std::clamp(static_cast<int>((r - 128) * factor + 128), 0, 255);
            g = std::clamp(static_cast<int>((g - 128) * factor + 128), 0, 255);
            b = std::clamp(static_cast<int>((b - 128) * factor + 128), 0, 255);

            row[x] = qRgb(r, g, b);
        }
    }

    return result;
}










