#include "exposurefilter.h"

ExposureFilter::ExposureFilter(int exposure)
    : m_exposure {exposure} {}




QImage ExposureFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};


    double exposure { 1.0 + getExposure() / 100.0};
    int height {result.height()};
    int width {result.width()};


    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {
            QRgb px {row[x]};

            int r {static_cast<int>(qRed(px) * exposure)};
            int g {static_cast<int>(qGreen(px) * exposure)};
            int b {static_cast<int>(qBlue(px) * exposure)};

            row[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)

                );
        }
    }

    return result;
}

bool ExposureFilter::isActive() const {
    return m_exposure != 0;
}

int ExposureFilter::getExposure() const {
    return m_exposure;
}

void ExposureFilter::setExposure(int exposure) {
    m_exposure = exposure;
}


std::unique_ptr<ImageFilter> ExposureFilter::clone() const {
    return std::make_unique<ExposureFilter>(*this);
}
