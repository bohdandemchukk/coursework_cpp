#include "blurfilter.h"
#include "gaussianblurutil.h"


BlurFilter::BlurFilter(int blur)
    : m_blur{blur} {}

int BlurFilter::getBlur() const {
    return m_blur;
}

void BlurFilter::setBlur(int blur) {
    m_blur = blur;
}


bool BlurFilter::isActive() const {
    return m_blur > 0;
}

QImage BlurFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    double sigma {getBlur() * 0.4};

    return GaussianBlurUtil::apply(input, sigma);
}



std::unique_ptr<ImageFilter> BlurFilter::clone() const {
    return std::make_unique<BlurFilter>(*this);
}
