#ifndef ERASERTOOL_H
#define ERASERTOOL_H

#include "brushtool.h"

class EraserTool : public BrushTool
{
public:
    explicit EraserTool(QImage* targetImage, std::function<void()> updateCallback);

protected:
    void paintStroke(QPainter& painter, const QPoint& from, const QPoint& to) override;
};

#endif // ERASERTOOL_H
