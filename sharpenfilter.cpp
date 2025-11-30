#include "sharpenfilter.h"
#include "gaussianblurutil.h"


SharpenFilter::SharpenFilter(int sharpness)
    : m_sharpness {sharpness} {}

int SharpenFilter::getSharpness() const {
    return m_sharpness;
}

void SharpenFilter::setSharpness(int sharpness) {
    m_sharpness = sharpness;
}

bool SharpenFilter::isActive() const {
    return m_sharpness > 0;
}

QImage SharpenFilter::apply(const QImage& input) const {

    if (!isActive()) return input;


    int sharpness {getSharpness()};

    QImage result {input.copy()};
    QImage blurred {GaussianBlurUtil::apply(input, 1.0)};

    int height {result.height()};
    int width {result.width()};

    for (int y {0}; y < height; y++) {

        const QRgb* src {reinterpret_cast<const QRgb*>(input.scanLine(y))};
        QRgb* blr {reinterpret_cast<QRgb*>(blurred.scanLine(y))};
        QRgb* dst {reinterpret_cast<QRgb*>(result.scanLine(y))};

        for (int x{0}; x < width; x++) {
            int r { (1 + sharpness) * qRed(src[x]) - sharpness * qRed(blr[x]) };
            int g { (1 + sharpness) * qGreen(src[x]) - sharpness * qGreen(blr[x]) };
            int b { (1 + sharpness) * qBlue(src[x]) - sharpness * qBlue(blr[x]) };

            dst[x] = qRgb(
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255)
                );
        }


    }
    return result;
}


