#include "brushtool.h"

#include <QPainter>
#include <QSize>

BrushTool::BrushTool(QImage* targetImage, std::function<void()> updateCallback, MyGraphicsView* view)
    : Tool(targetImage, std::move(updateCallback), view) {
    qDebug() << m_view;
}

void BrushTool::onMousePress(const QPoint& imagePos, Qt::MouseButton button)
{


    if (button != Qt::LeftButton) return;

    m_view->setMouseTracking(true);
    beginStroke(imagePos);
}

void BrushTool::onMouseMove(const QPoint& imagePos, Qt::MouseButtons buttons)
{
    if (!m_drawing || !(buttons & Qt::LeftButton)) return;
    continueStroke(imagePos);
}

std::unique_ptr<Command> BrushTool::onMouseRelease(const QPoint& imagePos, Qt::MouseButton button)
{
    if (!m_drawing || button != Qt::LeftButton) return nullptr;
    continueStroke(imagePos);
    m_view->setMouseTracking(false);
    return endStroke();
}

void BrushTool::paintStroke(QPainter& painter, const QPoint& from, const QPoint& to)
{
    QPen pen(color(), brushSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawLine(from, to);
}

void BrushTool::beginStroke(const QPoint& pos)
{
    if (!targetImage()) return;

    m_drawing = true;
    m_lastPos = pos;
    m_boundingRect = QRect(pos, QSize(1, 1)).adjusted(-brushSize(), -brushSize(), brushSize(), brushSize());
    m_mask = QImage(targetImage()->size(), QImage::Format_Alpha8);
    m_mask.fill(0);
    m_preStrokeSnapshot = targetImage()->copy();

    continueStroke(pos);
}

void BrushTool::continueStroke(const QPoint& pos)
{
    if (!targetImage()) return;

    QPainter painter(targetImage());
    painter.setRenderHint(QPainter::Antialiasing, true);
    paintStroke(painter, m_lastPos, pos);

    QPainter maskPainter(&m_mask);
    maskPainter.setRenderHint(QPainter::Antialiasing, true);
    QPen maskPen(QColor(255, 255, 255), brushSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    maskPainter.setPen(maskPen);
    maskPainter.drawLine(m_lastPos, pos);

    m_boundingRect = m_boundingRect.united(expandedRect(m_lastPos, pos));
    m_lastPos = pos;

    requestUpdate();
}

std::unique_ptr<Command> BrushTool::endStroke()
{
    if (!targetImage()) {
        m_drawing = false;
        return nullptr;
    }

    m_drawing = false;
    QRect rect = m_boundingRect.intersected(targetImage()->rect());
    if (rect.isEmpty()) return nullptr;

    QImage before = m_preStrokeSnapshot.copy(rect);
    QImage maskSection = m_mask.copy(rect);

    QImage after = targetImage()->copy(rect);

    m_mask = QImage();
    m_boundingRect = QRect();
    m_preStrokeSnapshot = QImage();

    return std::make_unique<StrokeCommand>(
        targetImage(),
        rect,
        before,
        after,
        maskSection,
        [this]() { requestUpdate(); }
        );
}

QRect BrushTool::expandedRect(const QPoint& a, const QPoint& b) const
{
    QRect base = QRect(a, b).normalized();
    int margin = brushSize();
    return base.adjusted(-margin, -margin, margin, margin);
}
