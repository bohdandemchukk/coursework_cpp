#include "erasertool.h"

EraserTool::EraserTool(QImage* targetImage, std::function<void()> updateCallback, MyGraphicsView* view)
    : BrushTool(targetImage, std::move(updateCallback), view)
{}

void EraserTool::paintStroke(QPainter& painter, const QPoint& from, const QPoint& to)
{
    painter.setCompositionMode(QPainter::CompositionMode_Clear);

    QPen pen(Qt::transparent, brushSize(),
             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter.setPen(pen);
    painter.drawLine(from, to);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
}
