#include "temperaturefilter.h"



TemperatureFilter::TemperatureFilter(int temperature)
    : m_temperature{temperature} {}


QImage TemperatureFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};


    int temperature {getTemperature()};
    int height {result.height()};
    int width {result.width()};


    int deltaR {static_cast<int>(temperature * 0.6)};
    int deltaG {static_cast<int>(temperature * 0.2)};
    int deltaB {static_cast<int>(-temperature * 0.6)};

    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {

            row[x] = qRgb(
                std::clamp(qRed(row[x]) + deltaR, 0, 255),
                std::clamp(qGreen(row[x]) + deltaG, 0, 255),
                std::clamp(qBlue(row[x]) + deltaB, 0, 255)

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




