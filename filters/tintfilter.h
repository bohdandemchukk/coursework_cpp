#ifndef TINTFILTER_H
#define TINTFILTER_H



#include "imagefilter.h"
#include <QImage>


class TintFilter: public ImageFilter
{
public:
    TintFilter(int tint);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getTint() const;

    void setTint(int tint);

private:

    int m_tint{};

};





#endif // TINTFILTER_H
