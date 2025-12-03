#ifndef HIGHLIGHTFILTER_H
#define HIGHLIGHTFILTER_H

#include "imagefilter.h"
#include <QImage>


class HighlightFilter: public ImageFilter
{
public:
    HighlightFilter(int highlight);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    int getHighlight() const;

    void setHighlight(int highlight);

private:

    int m_highlight{};

};

#endif // HIGHLIGHTFILTER_H
