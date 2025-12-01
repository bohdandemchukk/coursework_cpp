#include "temperaturefilter.h"



TemperatureFilter::TemperatureFilter(int temperature)
    : m_temperature{temperature} {}


QImage TemperatureFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};


    int temperature {getTemperature()};
    int height {result.height()};
    int width {result.width()};


    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {
            QRgb px {row[x]};

            int r {static_cast<int>(qRed(px) + temperature * 0.6)};
            int g {static_cast<int>(qGreen(px) + temperature * 0.2)};
            int b {static_cast<int>(qBlue(px) - temperature * 0.6)};

            row[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)

            );
        }
    }

    return result;
}

bool TemperatureFilter::isActive() const {
    return m_temperature != 0;
}

int TemperatureFilter::getTemperature() const {
    return m_temperature;
}

void TemperatureFilter::setTemperature(int temperature) {
    m_temperature = temperature;
}




