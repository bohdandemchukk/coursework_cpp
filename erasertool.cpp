#include "erasertool.h"

EraserTool::EraserTool(QImage* targetImage, std::function<void()> updateCallback)
    : BrushTool(targetImage, std::move(updateCallback))
{}

void EraserTool::paintStroke(QPainter& painter, const QPoint& from, const QPoint& to)
{
    QPen pen(QColor(0, 0, 0, 0), brushSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.drawLine(from, to);
}
