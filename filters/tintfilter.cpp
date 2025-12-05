#include "tintfilter.h"

TintFilter::TintFilter(int tint)
    : m_tint{tint} {}


QImage TintFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage result {input.copy()};


    double tint {static_cast<double>(getTint()) / 100.0};
    int height {result.height()};
    int width {result.width()};

    int deltaR {static_cast<int>(tint * 35.0)};
    int deltaG {static_cast<int>(tint * 15.0)};
    int deltaB {static_cast<int>(tint * 35.0)};

    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {

            row[x] = qRgb(
                std::clamp(qRed(row[x]) + deltaR, 0, 255),
                std::clamp(qGreen(row[x]) - deltaG, 0, 255),
                std::clamp(qBlue(row[x]) + deltaB, 0, 255)

                );
        }
    }

    return result;
}

bool TintFilter::isActive() const {
    return m_tint != 0;
}

int TintFilter::getTint() const {
    return m_tint;
}

void TintFilter::setTint(int tint) {
    m_tint = tint;
}




