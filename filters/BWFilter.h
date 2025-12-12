#ifndef BLACKWHITE_H
#define BLACKWHITE_H

#include "ImageFilter.h"
#include <QImage>

class BWFilter: public ImageFilter
{
public:

    BWFilter(bool enabled);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    bool getEnabled() const;
    void setEnabled();

    std::unique_ptr<ImageFilter> clone() const override;

private:
    bool m_enabled{};
};

#endif // BLACKWHITE_H
