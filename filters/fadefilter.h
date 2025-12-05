#ifndef FADEFILTER_H
#define FADEFILTER_H

#include "ImageFilter.h"
#include <QImage>

class FadeFilter : public ImageFilter
{
public:
    FadeFilter(int fade);

    QImage apply(const QImage &input) const override;
    bool isActive() const override;

    int getFade() const;
    void setFade(int fade);

private:
    int m_fade {};
};

#endif // FADEFILTER_H
