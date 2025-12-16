#ifndef GRAINFILTER_H
#define GRAINFILTER_H

#include "ImageFilter.h"
#include <QImage>

class GrainFilter : public ImageFilter
{
public:
    explicit GrainFilter(int grain);

    QImage apply(const QImage &input) const override;
    bool isActive() const override;

    int getGrain() const;
    void setGrain(int grain);
    std::unique_ptr<ImageFilter> clone() const override;
private :
    int m_grain {};
};

#endif // GRAINFILTER_H
