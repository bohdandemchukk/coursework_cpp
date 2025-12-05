#ifndef GAMMAFILTER_H
#define GAMMAFILTER_H

#include "imagefilter.h"
#include <QImage>


class GammaFilter: public ImageFilter
{
public:
    GammaFilter(int gamma);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getGamma() const;

    void setGamma(int gamma);

private:

    int m_gamma{};

};


#endif // GAMMAFILTER_H



