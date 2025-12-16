#ifndef FASTBLURFILTER_H
#define FASTBLURFILTER_H
#include "imagefilter.h"
#include <QImage>


class FastBlurFilter: public ImageFilter
{
public:
    FastBlurFilter(int blur);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getBlur() const;

    void setBlur(int blur);

    std::unique_ptr<ImageFilter> clone() const override;

private:

    int m_blur{};
};

#endif // FASTBLURFILTER_H
