#ifndef FLIPFILTER_H
#define FLIPFILTER_H

#include "ImageFilter.h"
#include <QImage>

class FlipFilter: public ImageFilter
{


public:
    enum class Direction {
        Horizontal,
        Vertical
    };

    FlipFilter(Direction direction, bool enabled);

    QImage apply(const QImage& input) const override;

    bool isActive() const override;

    Direction getDirection() const;
    void setDirection(Direction direction);

    bool getEnabled() const;
    void setEnabled();
    std::unique_ptr<ImageFilter> clone() const override;


private:
    Direction m_direction {};
    bool m_enabled{};
};

#endif // FLIPFILTER_H



