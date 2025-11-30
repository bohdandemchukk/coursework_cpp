#ifndef SATURATIONFILTER_H
#define SATURATIONFILTER_H


#include "ImageFilter.h"
#include <QImage>


class SaturationFilter: public ImageFilter
{
public:
    SaturationFilter(int saturation);
    QImage apply(const QImage& input) const override;
    bool isActive() const override;
    int getSaturation() const;
    void setSaturation(int saturation);

private:
    int m_saturation {};
};

#endif // SATURATIONFILTER_H


