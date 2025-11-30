#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <QImage>

class ImageFilter {
public:
    virtual ~ImageFilter() = default;
    virtual QImage apply(const QImage& input) const = 0;
    virtual bool isActive() const = 0;
};

#endif // IMAGEFILTER_H
