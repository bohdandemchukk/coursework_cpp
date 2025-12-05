#ifndef SPLITTONINGFILTER_H
#define SPLITTONINGFILTER_H

#include "ImageFilter.h"
#include <QImage>

class SplitToningFilter : public ImageFilter
{
public:
    explicit SplitToningFilter(int splitToning);

    QImage apply(const QImage &input) const override;
    bool isActive() const override;

    int getSplitToning() const;
    void setSplitToning(int splitToning);

private:
    int m_splitToning {};
};

#endif // SPLITTONINGFILTER_H
