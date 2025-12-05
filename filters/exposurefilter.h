#ifndef EXPOSUREFILTER_H
#define EXPOSUREFILTER_H

#include "imagefilter.h"
#include <QImage>


class ExposureFilter: public ImageFilter
{
public:
    ExposureFilter(int exposure);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getExposure() const;

    void setExposure(int exposure);

private:

    int m_exposure{};

};



#endif // EXPOSUREFILTER_H
