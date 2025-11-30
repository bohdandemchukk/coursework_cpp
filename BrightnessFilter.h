#ifndef BRIGHTNESSFILTER_H
#define BRIGHTNESSFILTER_H


#include "ImageFilter.h"
#include <QImage>


class BrightnessFilter: public ImageFilter
{
public:
    BrightnessFilter(int brightness);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getBrightness() const;
    void setBrightness(int brightness);




private:
    int m_brightness{};
};

#endif // BRIGHTNESSFILTER_H
