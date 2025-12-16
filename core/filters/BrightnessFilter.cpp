#include "BrightnessFilter.h"

BrightnessFilter::BrightnessFilter(int brightness)
    : m_brightness{brightness} {}


bool BrightnessFilter::isActive() const {
    return m_brightness != 0;
}

int BrightnessFilter::getBrightness() const {
    return m_brightness;
}

void BrightnessFilter::setBrightness(int brightness) {
    m_brightness = brightness;
}

QImage BrightnessFilter::apply(const QImage& input) const  {
    if (!isActive()) return input;

    QImage result {input.copy()};
    int height {result.height()};
    int width {result.width()};

    int brightness {getBrightness()};

    for (int y {0}; y < height; y++) {
        QRgb *row = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x {0}; x < width; x++) {
            QRgb px = row[x];

            int r = std::clamp(qRed(px) + brightness, 0, 255);
            int g = std::clamp(qGreen(px) + brightness, 0, 255);
            int b = std::clamp(qBlue(px) + brightness, 0, 255);

            row [x] = qRgb(r, g, b);
        }
    }

    return result;
}


std::unique_ptr<ImageFilter> BrightnessFilter::clone() const  {
    return std::make_unique<BrightnessFilter>(*this);
}
