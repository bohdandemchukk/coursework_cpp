#include "grainfilter.h"

#include <QRandomGenerator>

GrainFilter::GrainFilter(int grain)
    : m_grain{grain} {}

bool GrainFilter::isActive() const {
    return m_grain != 0;
}

int GrainFilter::getGrain() const {
    return m_grain;
}

void GrainFilter::setGrain(int grain) {
    m_grain = grain;
}

QImage GrainFilter::apply(const QImage &input) const {
    if (!isActive()) return input;

    QImage result = input.copy();
    const int width = result.width();
    const int height = result.height();

    const int grain = std::clamp(getGrain(), -100, 100);

    for (int y = 0; y < height; ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(result.scanLine(y));

        for (int x = 0; x < width; ++x) {
            QRgb pix = row[x];
            int offset = QRandomGenerator::global()->bounded(-grain, grain + 1);

            int r = std::clamp(qRed(pix) + offset, 0, 255);
            int g = std::clamp(qGreen(pix) + offset, 0, 255);
            int b = std::clamp(qBlue(pix) + offset, 0, 255);

            row[x] = qRgb(r, g, b);
        }
    }

    return result;
}

std::unique_ptr<ImageFilter> GrainFilter::clone() const {
    return std::make_unique<GrainFilter>(*this);
}
