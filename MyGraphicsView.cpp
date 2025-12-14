#include "MyGraphicsView.h"

#include <QScrollBar>
#include <QPainter>
#include <algorithm>

#include "tool.h"
#include "layercommands.h"

namespace {
constexpr qreal kHandleSize = 8.0;
}

MyGraphicsView::MyGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent),
    rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
    setMouseTracking(true);
}


void MyGraphicsView::setPixmap(const QPixmap& pixmap)
{
    if (!scene())
        setScene(new QGraphicsScene(this));

    if (!pixmapItem) {
        pixmapItem = scene()->addPixmap(pixmap);
    } else {
        pixmapItem->setPixmap(pixmap);
    }

    scene()->setSceneRect(pixmap.rect());
}


void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    const qreal factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    scale(factor, factor);
    emit zoomChanged(transform().m11());
}


void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    const QPointF scenePos = mapToScene(event->pos());

    // --- PAN ---
    if (getPanMode() && event->button() == Qt::LeftButton) {
        m_dragContext = DragContext::Pan;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // --- CROP ---
    if (getCropMode() && event->button() == Qt::LeftButton) {
        m_dragContext = DragContext::Crop;
        setCropStart(event->pos());
        rubberBand->setGeometry(QRect(getCropStart(), QSize()));
        rubberBand->show();
        event->accept();
        return;
    }

    // --- TOOL HAS PRIORITY ---
    if (m_activeTool && event->button() == Qt::LeftButton) {
        m_dragContext = DragContext::Paint;
        QPoint imgPos = mapToActiveLayerImage(scenePos);
        m_activeTool->onMousePress(imgPos, event->button());
        event->accept();
        return;
    }

    // --- MOVE / SCALE (ONLY IF NO TOOL) ---
    if (event->button() == Qt::LeftButton && m_layerManager) {
        int hitIndex = -1;
        auto pixelLayer = hitTestLayers(scenePos, hitIndex);
        if (pixelLayer) {
            m_layerManager->setActiveLayerIndex(hitIndex);

            m_dragStartScene = scenePos;
            m_initialOffset  = pixelLayer->offset();
            m_initialScale   = pixelLayer->scale();

            QSizeF imgSize = pixelLayer->image().size();
            m_initialCenter = m_initialOffset +
                              QPointF(imgSize.width() * m_initialScale / 2.0,
                                      imgSize.height() * m_initialScale / 2.0);

            int handle = hitHandle(scenePos);
            if (handle >= 0) {
                m_dragContext = DragContext::ScaleLayer;
                m_dragLayerIndex = hitIndex;
                event->accept();
                return;
            }

            if (pixelLayer->bounds().contains(scenePos)) {
                m_dragContext = DragContext::MoveLayer;
                m_dragLayerIndex = hitIndex;
                event->accept();
                return;
            }
        }
    }

    m_dragContext = DragContext::None;
    QGraphicsView::mousePressEvent(event);
}


void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF scenePos = mapToScene(event->pos());

    switch (m_dragContext) {

    case DragContext::Crop:
        rubberBand->setGeometry(QRect(getCropStart(), event->pos()).normalized());
        event->accept();
        return;

    case DragContext::Pan: {
        QPoint delta = m_lastPanPoint - event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
        m_lastPanPoint = event->pos();
        event->accept();
        return;
    }

    case DragContext::Paint: {
        QPoint imgPos = mapToActiveLayerImage(scenePos);
        m_activeTool->onMouseMove(imgPos, event->buttons());

        if (m_layerManager)
            m_layerManager->markDirty();

        event->accept();
        return;
    }


    case DragContext::MoveLayer: {
        auto layer = std::dynamic_pointer_cast<PixelLayer>(
            m_layerManager->layerAt(m_dragLayerIndex));
        if (!layer) return;

        QPointF delta = scenePos - m_dragStartScene;
        layer->setOffset(m_initialOffset + delta);
        viewport()->update(); // ❗ НЕ notifyLayerChanged
        event->accept();
        return;
    }

    case DragContext::ScaleLayer: {
        auto layer = std::dynamic_pointer_cast<PixelLayer>(
            m_layerManager->layerAt(m_dragLayerIndex));
        if (!layer) return;

        QPointF v0 = m_dragStartScene - m_initialCenter;
        QPointF v1 = scenePos - m_initialCenter;
        if (v0.manhattanLength() == 0) return;

        qreal factor = v1.manhattanLength() / v0.manhattanLength();
        float newScale = std::max(0.01f, float(m_initialScale * factor));

        QSizeF imgSize = layer->image().size();
        QPointF newOffset = m_initialCenter -
                            QPointF(imgSize.width() * newScale / 2.0,
                                    imgSize.height() * newScale / 2.0);

        layer->setScale(newScale);
        layer->setOffset(newOffset);
        viewport()->update();
        event->accept();
        return;
    }

    default:
        break;
    }

    QGraphicsView::mouseMoveEvent(event);
}



void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_dragContext == DragContext::MoveLayer ||
        m_dragContext == DragContext::ScaleLayer)
    {
        m_layerManager->notifyLayerChanged(); // ОДИН раз
    }

    if (m_dragContext == DragContext::Paint && m_activeTool) {
        QPoint imgPos = mapToActiveLayerImage(mapToScene(event->pos()));
        if (auto cmd = m_activeTool->onMouseRelease(imgPos, event->button()))
            emit commandReady(cmd.release());
    }

    m_dragContext = DragContext::None;
    QGraphicsView::mouseReleaseEvent(event);
}


QPoint MyGraphicsView::mapToActiveLayerImage(const QPointF& scenePos) const
{
    if (!m_layerManager)
        return {};

    auto layer = std::dynamic_pointer_cast<PixelLayer>(
        m_layerManager->activeLayer());

    if (!layer)
        return {};

    QPointF local = (scenePos - layer->offset()) / layer->scale();

    int x = qBound(0, int(local.x()), layer->image().width()  - 1);
    int y = qBound(0, int(local.y()), layer->image().height() - 1);

    return QPoint(x, y);
}

QRectF MyGraphicsView::activeLayerBounds() const
{
    if (!m_layerManager)
        return {};

    auto layer = std::dynamic_pointer_cast<PixelLayer>(
        m_layerManager->activeLayer());

    return layer ? layer->bounds() : QRectF{};
}

QVector<QRectF> MyGraphicsView::handleRects(const QRectF& bounds) const
{
    QVector<QRectF> rects;
    if (bounds.isNull())
        return rects;

    const qreal half = kHandleSize / 2.0;
    rects << QRectF(bounds.topLeft()     - QPointF(half, half), QSizeF(kHandleSize, kHandleSize))
          << QRectF(bounds.topRight()    - QPointF(half, half), QSizeF(kHandleSize, kHandleSize))
          << QRectF(bounds.bottomRight() - QPointF(half, half), QSizeF(kHandleSize, kHandleSize))
          << QRectF(bounds.bottomLeft()  - QPointF(half, half), QSizeF(kHandleSize, kHandleSize));

    return rects;
}

int MyGraphicsView::hitHandle(const QPointF& scenePos) const
{
    auto rects = handleRects(activeLayerBounds());
    for (int i = 0; i < rects.size(); ++i) {
        if (rects[i].contains(scenePos))
            return i;
    }
    return -1;
}

std::shared_ptr<PixelLayer>
MyGraphicsView::hitTestLayers(const QPointF& scenePos, int& outIndex) const
{
    outIndex = -1;
    if (!m_layerManager)
        return nullptr;

    for (int i = m_layerManager->layerCount() - 1; i >= 0; --i) {
        auto pixel = std::dynamic_pointer_cast<PixelLayer>(
            m_layerManager->layerAt(i));

        if (pixel && pixel->isVisible() &&
            pixel->bounds().contains(scenePos))
        {
            outIndex = i;
            return pixel;
        }
    }
    return nullptr;
}

void MyGraphicsView::drawForeground(QPainter* painter, const QRectF&)
{
    if (m_activeTool) return;
    auto bounds = activeLayerBounds();
    if (bounds.isNull())
        return;

    painter->save();
    painter->setPen(QPen(Qt::white, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(bounds);

    painter->setBrush(Qt::white);
    for (const auto& r : handleRects(bounds))
        painter->drawRect(r);

    painter->restore();
}
