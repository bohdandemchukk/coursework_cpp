#include "vibrancefilter.h"
#include "rgbhsvutil.h"

VibranceFilter::VibranceFilter(int vibrance)
    : m_vibrance{vibrance} {}


QImage VibranceFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};


    double vibrance {getVibrance() / 100.0};
    int height {result.height()};
    int width {result.width()};


    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {


            double r { qRed(row[x]) / 255.0 };
            double g { qGreen(row[x]) / 255.0 };
            double b { qBlue(row[x]) / 255.0 };


            double h{}, s{}, v{};

            RgbHsvUtil::rgb2hsv(r, g, b, h, s, v);


            double saturationWeight = (1.0 - s) * (1.0 - s);
            double boost = vibrance * saturationWeight * 0.5;

            if (h >= 0 && h <= 60) {
                boost *= 0.3;
            }

            s += boost;
            s = std::clamp(s, 0.0, 1.0);


            row[x] = RgbHsvUtil::hsv2rgb(h, s, v);
        }
    }

    return result;
}

bool VibranceFilter::isActive() const {
    return m_vibrance != 0;
}

int VibranceFilter::getVibrance() const {
    return m_vibrance;
}

void VibranceFilter::setVibrance(int vibrance) {
    m_vibrance = vibrance;
}




