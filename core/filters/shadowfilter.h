#ifndef SHADOWFILTER_H
#define SHADOWFILTER_H

#include "imagefilter.h"
#include <QImage>


class ShadowFilter: public ImageFilter
{
public:
    ShadowFilter(int shadow);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getShadow() const;

    void setShadow(int shadow);
    std::unique_ptr<ImageFilter> clone() const override;
private:

    int m_shadow{};

};





#endif // SHADOWFILTER_H
