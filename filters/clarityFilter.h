#ifndef CLARITYFILTER_H
#define CLARITYFILTER_H

#include "ImageFilter.h"
#include <QImage>


class ClarityFilter: public ImageFilter
{
public:
    ClarityFilter(int clarity);
    QImage apply(const QImage& input) const override;
    bool isActive() const override;
    int getClarity() const;
    void setClarity(int clarity);
    std::unique_ptr<ImageFilter> clone() const override;
private:
    int m_clarity {};
};

#endif // CLARITYFILTER_H
