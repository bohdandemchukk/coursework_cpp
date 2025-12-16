#include "fadefilter.h"

FadeFilter::FadeFilter(int fade)
    : m_fade{fade} {}

bool FadeFilter::isActive() const {
    return m_fade!= 0;
}

int FadeFilter::getFade() const {
    return m_fade;
}

void FadeFilter::setFade(int fade) {
    m_fade = fade;
}

QImage FadeFilter::apply(const QImage &input) const {
    if (!isActive()) return input;

    QImage result = input.copy();
    const double factor = std::clamp(getFade(), 0, 100) / 100.0;

    const int width = result.width();
    const int height = result.height();

    for (int y = 0; y < height; ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(result.scanLine(y));

        for (int x = 0; x < width; ++x) {
            const QRgb pix = row[x];

            const int r = static_cast<int>(qRed(pix) * (1.0 - factor) + 255.0 * factor);
            const int g = static_cast<int>(qGreen(pix) * (1.0 - factor) + 255.0 * factor);
            const int b = static_cast<int>(qBlue(pix) * (1.0 - factor) + 255.0 * factor);

            row[x] = qRgb(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255));
        }
    }

    return result;
}


std::unique_ptr<ImageFilter> FadeFilter::clone() const {
    return std::make_unique<FadeFilter>(*this);
}
