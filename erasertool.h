#ifndef ERASERTOOL_H
#define ERASERTOOL_H

#include "brushtool.h"
#include <QPen>
#include <QPainter>

class EraserTool : public BrushTool
{
public:
    explicit EraserTool(QImage* targetImage, std::function<void()> updateCallback, MyGraphicsView* view);

protected:
    void paintStroke(QPainter& painter, const QPoint& from, const QPoint& to) override;
};

#endif // ERASERTOOL_H
