#ifndef VIBRANCEFILTER_H
#define VIBRANCEFILTER_H

#include "imagefilter.h"
#include <QImage>


class VibranceFilter: public ImageFilter
{
public:
    VibranceFilter(int vibrance);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getVibrance() const;

    void setVibrance(int vibrance);

private:

    int m_vibrance{};

};


#endif // VIBRANCEFILTER_H
