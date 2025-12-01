#include "gammafilter.h"

GammaFilter::GammaFilter(int gamma)
    : m_gamma{gamma} {}





QImage GammaFilter::apply(const QImage& input) const {

    if (!isActive()) return input;

    QImage result {input.copy()};


    double gamma { 1.0 + getGamma() / 100.0};
    int height {result.height()};
    int width {result.width()};


    for (int y{0}; y < height; y++) {
        QRgb* row {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {
            QRgb px {row[x]};

            int r {static_cast<int>(255.0 * std::pow(qRed(px) / 255.0, gamma))};
            int g {static_cast<int>(255.0 * std::pow(qGreen(px) / 255.0, gamma))};
            int b {static_cast<int>(255.0 * std::pow(qBlue(px) / 255.0, gamma))};

            row[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)

                );
        }
    }

    return result;
}

bool GammaFilter::isActive() const {
    return m_gamma != 0;
}

int GammaFilter::getGamma() const {
    return m_gamma;
}

void GammaFilter::setGamma(int gamma) {
    m_gamma = gamma;
}
