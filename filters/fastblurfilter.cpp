#include "fastblurfilter.h"
#include "fastblur.h"


FastBlurFilter::FastBlurFilter(int blur)
    : m_blur{blur} {}

int FastBlurFilter::getBlur() const {
    return m_blur;
}

void FastBlurFilter::setBlur(int blur) {
    m_blur = blur;
}


bool FastBlurFilter::isActive() const {
    return m_blur > 0;
}

QImage FastBlurFilter::apply(const QImage& input) const {
    if (!isActive()) return input;

    double sigma {getBlur() * 0.4};

    return FastBlur::apply(input, sigma);
}



std::unique_ptr<ImageFilter> FastBlurFilter::clone() const {
    return std::make_unique<FastBlurFilter>(*this);
}
