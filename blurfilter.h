#ifndef BLURFILTER_H
#define BLURFILTER_H


#include "imagefilter.h"
#include <QImage>


class BlurFilter: public ImageFilter
{
public:
    BlurFilter(int blur);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getBlur() const;

    void setBlur(int blur);

private:

    int m_blur{};
};

#endif // BLURFILTER_H



