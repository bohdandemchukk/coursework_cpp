#include "saturationfilter.h"
#include "rgbhsvutil.h"

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
    QImage result = input.copy();

    int width = result.width();
    int height = result.height();

    for (int y = 0; y < height; ++y) {
        QRgb* row = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < width; ++x) {
            QRgb pix = row[x];

            double r = qRed(pix)   / 255.0;
            double g = qGreen(pix) / 255.0;
            double b = qBlue(pix)  / 255.0;

            double h, s, v;
            RgbHsvUtil::rgb2hsv(r, g, b, h, s, v);

            s = std::clamp(s * factor, 0.0, 1.0);

            row[x] = RgbHsvUtil::hsv2rgb(h, s, v);
        }
    }

    return result;
}









