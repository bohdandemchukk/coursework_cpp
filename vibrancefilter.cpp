#include "vibrancefilter.h"
#include "rgbhsvutil.h"
#include <cmath>

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
            int red = qRed(row[x]);
            int green = qGreen(row[x]);
            int blue = qBlue(row[x]);

            double r = red / 255.0;
            double g = green / 255.0;
            double b = blue / 255.0;

            // КРИТИЧНО: перевіряємо в RGB просторі спочатку!
            // Якщо колір майже сірий (різниця між каналами мала)
            int maxChannel = std::max({red, green, blue});
            int minChannel = std::min({red, green, blue});
            int diff = maxChannel - minChannel;

            // Пропускаємо майже нейтральні кольори (біле/сіре/чорне світло)
            if (diff < 20) {  // Різниця менше 20 = майже сірий
                continue;
            }

            double h{}, s{}, v{};
            RgbHsvUtil::rgb2hsv(r, g, b, h, s, v);

            // Додаткова перевірка після конвертації
            if (s < 0.08) {
                continue;
            }

            // Пропускаємо дуже світлі пікселі з низькою насиченістю
            if (v > 0.88 && s < 0.2) {
                continue;
            }

            // Формула vibrance
            double saturationWeight = 1.0 - s;
            double boost = vibrance * saturationWeight * 0.6;

            // Захист шкіряних тонів
            if (h >= 5 && h <= 45) {
                boost *= 0.4;
            }
            else if ((h >= 0 && h <= 5) || (h >= 345 && h <= 360)) {
                boost *= 0.5;
            }
            else if (h >= 45 && h <= 65) {
                boost *= 0.5;
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
