#include "tintfilter.h"

TintFilter::TintFilter(int tint)
    : m_tint{tint} {}


QImage TintFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};


    double tint {static_cast<double>(getTint()) / 100.0};
    int height {result.height()};
    int width {result.width()};


    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {
            QRgb px {row[x]};

            int r {static_cast<int>(qRed(px) + tint * 35)};
            int g {static_cast<int>(qGreen(px) - tint * 15)};
            int b {static_cast<int>(qBlue(px) + tint * 35)};

            row[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)

                );
        }
    }

    return result;
}

bool TintFilter::isActive() const {
    return m_tint != 0;
}

int TintFilter::getTint() const {
    return m_tint;
}

void TintFilter::setTint(int tint) {
    m_tint = tint;
}




