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
    if (!targetImage() || !m_view) return;
    if (!isInsideImage(imagePos)) return;

    m_view->setMouseTracking(true);
    beginStroke(imagePos);
}


void BrushTool::onMouseMove(const QPoint& imagePos, Qt::MouseButtons buttons)
{
    if (!m_drawing) return;
    if (!(buttons & Qt::LeftButton)) return;
    if (!targetImage()) return;
    if (!isInsideImage(imagePos)) return;

    continueStroke(imagePos);
}


bool BrushTool::isInsideImage(const QPoint& p) const
{
    return targetImage() &&
           p.x() >= 0 &&
           p.y() >= 0 &&
           p.x() < targetImage()->width() &&
           p.y() < targetImage()->height();
}



std::unique_ptr<Command> BrushTool::onMouseRelease(const QPoint& imagePos, Qt::MouseButton button)
{
    if (!m_drawing || button != Qt::LeftButton)
        return nullptr;

    if (!targetImage() || !m_view) {
        m_drawing = false;
        return nullptr;
    }

    if (isInsideImage(imagePos))
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

    m_previewBuffer = targetImage()->copy();

    m_mask = QImage(targetImage()->size(), QImage::Format_Alpha8);
    m_mask.fill(0);

    m_boundingRect = QRect(pos, QSize(1, 1))
                         .adjusted(-brushSize(), -brushSize(),
                                   brushSize(), brushSize());

    m_preStrokeSnapshot = targetImage()->copy();

    continueStroke(pos);
}


void BrushTool::continueStroke(const QPoint& pos)
{
    if (!m_drawing) return;

    QPainter painter(&m_previewBuffer);
    painter.setRenderHint(QPainter::Antialiasing, true);
    paintStroke(painter, m_lastPos, pos);

    QPainter maskPainter(&m_mask);
    maskPainter.setRenderHint(QPainter::Antialiasing, true);
    QPen maskPen(Qt::white, brushSize(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    maskPainter.setPen(maskPen);
    maskPainter.drawLine(m_lastPos, pos);

    m_boundingRect = m_boundingRect.united(expandedRect(m_lastPos, pos));
    m_lastPos = pos;

    *targetImage() = m_previewBuffer;
    requestUpdate();
}




std::unique_ptr<Command> BrushTool::endStroke()
{
    if (!m_drawing) return nullptr;
    m_drawing = false;

    QRect rect = m_boundingRect.intersected(targetImage()->rect());
    if (rect.isEmpty()) return nullptr;

    QImage before = m_preStrokeSnapshot.copy(rect);
    QImage after  = m_previewBuffer.copy(rect);
    QImage mask   = m_mask.copy(rect);

    *targetImage() = m_previewBuffer;
    requestUpdate();

    return std::make_unique<StrokeCommand>(
        targetImage(),
        rect,
        before,
        after,
        mask,
        [this]() {
            if (m_view && m_view->layerManager())
                m_view->layerManager()->notifyChanged();
        }

        );
}


QRect BrushTool::expandedRect(const QPoint& a, const QPoint& b) const
{
    QRect base = QRect(a, b).normalized();
    int margin = brushSize();
    return base.adjusted(-margin, -margin, margin, margin);
}
