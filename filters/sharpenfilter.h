#ifndef SHARPENFILTER_H
#define SHARPENFILTER_H


#include "imagefilter.h"
#include <QImage>


class SharpenFilter: public ImageFilter
{
public:
    SharpenFilter(int sharpness);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getSharpness() const;

    void setSharpness(int sharpness);
    std::unique_ptr<ImageFilter> clone() const override;
private:

    int m_sharpness{};

};

#endif // SHARPENFILTER_H

