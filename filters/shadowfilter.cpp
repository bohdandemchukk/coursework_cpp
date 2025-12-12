#include "shadowfilter.h"


ShadowFilter::ShadowFilter(int shadow)
    : m_shadow{shadow} {}


QImage ShadowFilter::apply(const QImage& input) const {

    if (!isActive()) return input;

    QImage result {input.copy()};


    double shadowBoost { getShadow() / 100.0};
    int height {result.height()};
    int width {result.width()};


    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {
            QRgb px {row[x]};

            double luma {0.299 * qRed(px) + 0.587 * qGreen(px) + 0.114 * qBlue(px)};
            double factor {1.0 + shadowBoost * (1.0 - luma/255.0)};

            int r {static_cast<int>(qRed(px) * factor)};
            int g {static_cast<int>(qGreen(px) * factor)};
            int b {static_cast<int>(qBlue(px) * factor)};

            row[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)

                );
        }
    }

    return result;
}

bool ShadowFilter::isActive() const {
    return m_shadow != 0;
}

int ShadowFilter::getShadow() const {
    return m_shadow;
}

void ShadowFilter::setShadow(int shadow) {
    m_shadow = shadow;
}


std::unique_ptr<ImageFilter> ShadowFilter::clone() const {
    return std::make_unique<ShadowFilter>(*this);
}
