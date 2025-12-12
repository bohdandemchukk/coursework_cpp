#include "clarityFilter.h"
#include "gaussianblurutil.h"


ClarityFilter::ClarityFilter(int clarity)
    : m_clarity{clarity} {}

bool ClarityFilter::isActive() const {
    return m_clarity != 0;
}

int ClarityFilter::getClarity() const {
    return m_clarity;
}

void ClarityFilter::setClarity(int clarity) {
    m_clarity = clarity;
}

QImage ClarityFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    QImage blurred {GaussianBlurUtil::apply(input, 15)};
    QImage result {input.copy()};

    double clarity {getClarity() / 100.0};

    int height {result.height()};
    int width {result.width()};

    for (int y = 0; y < height; y++) {
        QRgb* row  { reinterpret_cast<QRgb*>(result.scanLine(y))};
        const QRgb* blurRow { reinterpret_cast<const QRgb*>(blurred.constScanLine(y))};

        for (int x = 0; x < width; x++) {

            double rOrig = qRed(row[x]) / 255.0;
            double gOrig = qGreen(row[x]) / 255.0;
            double bOrig = qBlue(row[x]) / 255.0;

            double rBlur = qRed(blurRow[x]) / 255.0;
            double gBlur = qGreen(blurRow[x]) / 255.0;
            double bBlur = qBlue(blurRow[x]) / 255.0;

            double rDetail = rOrig - rBlur;
            double gDetail = gOrig - gBlur;
            double bDetail = bOrig - bBlur;

            double r = rOrig + rDetail * clarity;
            double g = gOrig + gDetail * clarity;
            double b = bOrig + bDetail * clarity;

            row[x] = qRgb(
                std::clamp(int(r * 255), 0, 255),
                std::clamp(int(g * 255), 0, 255),
                std::clamp(int(b * 255), 0, 255)
                );
        }
    }

    return result;
}

std::unique_ptr<ImageFilter> ClarityFilter::clone() const {
    return std::make_unique<ClarityFilter>(*this);
}








