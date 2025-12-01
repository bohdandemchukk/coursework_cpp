#ifndef TEMPERATUREFILTER_H
#define TEMPERATUREFILTER_H

#include "imagefilter.h"
#include <QImage>


class TemperatureFilter: public ImageFilter
{
public:
    TemperatureFilter(int temperature);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getTemperature() const;

    void setTemperature(int temperature);

private:

    int m_temperature{};

};



#endif // TEMPERATUREFILTER_H
