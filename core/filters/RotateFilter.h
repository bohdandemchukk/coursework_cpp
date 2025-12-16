#ifndef ROTATEFILTER_H
#define ROTATEFILTER_H


#include "ImageFilter.h"
#include <QImage>
#include <QTransform>

class RotateFilter: public ImageFilter {
public:
    RotateFilter(int angle = 0);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;


    int getAngle() const;

    void setAngle(int angle);
    std::unique_ptr<ImageFilter> clone() const override;



private:
    int m_angle{};


};

#endif // ROTATEFILTER_H


