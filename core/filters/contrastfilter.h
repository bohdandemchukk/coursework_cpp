#ifndef CONTRASTFILTER_H
#define CONTRASTFILTER_H


#include "ImageFilter.h"
#include <QImage>


class ContrastFilter: public ImageFilter
{
public:
    ContrastFilter(int contrast);
    QImage apply(const QImage& input) const override;
    bool isActive() const override;
    int getContrast() const;
    void setContrast(int contrast);
    std::unique_ptr<ImageFilter> clone() const override;
private:
    int m_contrast {};
};




#endif // CONTRASTFILTER_H
