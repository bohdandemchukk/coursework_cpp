#include "splittoningfilter.h"


SplitToningFilter::SplitToningFilter(int splitToning)
    : m_splitToning{splitToning} {}

bool SplitToningFilter::isActive() const {
    return m_splitToning != 0;
}

int SplitToningFilter::getSplitToning() const {
    return m_splitToning;
}

void SplitToningFilter::setSplitToning(int splitToning) {
    m_splitToning = splitToning;
}

QImage SplitToningFilter::apply(const QImage &input) const {
    if (!isActive()) return input;

    QImage result = input.copy();

    const double factor = std::clamp(getSplitToning(), 0, 100) / 100.0;
    const QColor shadowTint{64, 160, 170};
    const QColor highlightTint{255, 199, 145};

    const int width = result.width();
    const int height = result.height();

    for (int y = 0; y < height; ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(result.scanLine(y));

        for (int x = 0; x < width; ++x) {
            const QRgb pix = row[x];
            const double luminance = qGray(pix) / 255.0;

            const double shadowWeight = 1.0 - luminance;
            const double highlightWeight = luminance;

            const double mixR = shadowTint.red() * shadowWeight + highlightTint.red() * highlightWeight;
            const double mixG = shadowTint.green() * shadowWeight + highlightTint.green() * highlightWeight;
            const double mixB = shadowTint.blue() * shadowWeight + highlightTint.blue() * highlightWeight;

            const int r = static_cast<int>(qRed(pix) * (1.0 - factor) + mixR * factor);
            const int g = static_cast<int>(qGreen(pix) * (1.0 - factor) + mixG * factor);
            const int b = static_cast<int>(qBlue(pix) * (1.0 - factor) + mixB * factor);

            row[x] = qRgb(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255));
        }
    }

    return result;
}
