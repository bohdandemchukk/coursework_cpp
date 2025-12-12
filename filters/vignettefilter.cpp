#include "vignettefilter.h"

VignetteFilter::VignetteFilter(int vignette)
    : m_vignette{vignette} {}

QImage VignetteFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};

    const double vignette {std::clamp(m_vignette / 100.0, 0.0, 2.0)};

    const int width {result.width()};
    const int height {result.height()};

    const double centerX {width / 2.0};
    const double centerY {height / 2.0};
    const double maxDistance {std::hypot(centerX, centerY)};

    for (int y {0}; y < height; ++y) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x {0}; x < width; ++x) {
            const double dx {x - centerX};
            const double dy {y - centerY};
            const double distanceRatio {std::hypot(dx, dy) / maxDistance};

            double factor {1.0 - vignette * distanceRatio * distanceRatio};
            factor = std::clamp(factor, 0.0, 1.0);

            const QRgb px {row[x]};
            const int r {static_cast<int>(qRed(px) * factor)};
            const int g {static_cast<int>(qGreen(px) * factor)};
            const int b {static_cast<int>(qBlue(px) * factor)};

            row[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)
                );
        }
    }

    return result;
}

bool VignetteFilter::isActive() const {
    return m_vignette != 0;
}

int VignetteFilter::getVignette() const {
    return m_vignette;
}

void VignetteFilter::setVignette(int vignette) {
    m_vignette = vignette;
}

std::unique_ptr<ImageFilter> VignetteFilter::clone() const {
    return std::make_unique<VignetteFilter>(*this);
}
