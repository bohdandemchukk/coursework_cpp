#include "MyGraphicsView.h"

#include <QScrollBar>
#include <QPainter>
#include <algorithm>
#include <QGraphicsDropShadowEffect>
#include "tool.h"
#include "layercommands.h"
#include "cropcommand.h"


namespace {
constexpr qreal kHandleSize = 12.0;
}

MyGraphicsView::MyGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent),
    m_scene(scene),
    rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);

}


void MyGraphicsView::setPixmap(const QPixmap& pixmap)
{


    if (!pixmapItem) {
        pixmapItem = m_scene->addPixmap(pixmap);

        auto* shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 120));
        pixmapItem->setGraphicsEffect(shadow);
    } else {
        pixmapItem->setPixmap(pixmap);
    }

    if (m_layerManager) {

        pixmapItem->setPos(m_layerManager->compositeOffset());
    } else {

        pixmapItem->setPos(0, 0);
    }



    m_scene->setSceneRect(pixmapItem->sceneBoundingRect());


}

void MyGraphicsView::clearPixmap()
{
    if (pixmapItem) {
        m_scene->removeItem(pixmapItem);
        delete pixmapItem;
        pixmapItem = nullptr;
    }
    m_scene->setSceneRect(QRectF());
}


void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QTransform t = transform();
    qreal factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    t.scale(factor, factor);
    setTransform(t);

    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    emit zoomChanged(t.m11());
}


void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
    const QPointF scenePos = mapToScene(event->pos());

    if (getPanMode() && event->button() == Qt::LeftButton) {
        m_dragContext = DragContext::Pan;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (getCropMode() && event->button() == Qt::LeftButton) {



        m_dragContext = DragContext::Crop;
        setCropStart(event->pos());
        rubberBand->setGeometry(QRect(getCropStart(), QSize()));
        rubberBand->show();
        event->accept();
        return;
    }

    if (m_activeTool && event->button() == Qt::LeftButton) {
        m_dragContext = DragContext::Paint;

        if (m_layerManager)
            m_layerManager->setPainting(true);

        QPoint imgPos = mapToActiveLayerImage(scenePos);
        m_activeTool->onMousePress(imgPos, event->button());
        event->accept();
        return;
    }


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
                m_scaleStartOffset = pixelLayer->offset();
                m_scaleStartScale = pixelLayer->scale();
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

        viewport()->update();
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

    if (m_dragContext == DragContext::Crop && m_layerManager)
    {
        rubberBand->hide();

        QRect viewRect = QRect(getCropStart(), event->pos()).normalized();
        QRectF sceneRect = mapToScene(viewRect).boundingRect();

        auto layer = std::dynamic_pointer_cast<PixelLayer>(
            m_layerManager->activeLayer());

        if (layer) {
            QRectF layerRect = layer->bounds();
            QRectF cropScene = sceneRect.intersected(layerRect);

            if (!cropScene.isEmpty()) {

                QRect cropImg(
                    int((cropScene.left() - layerRect.left()) / layer->scale()),
                    int((cropScene.top()  - layerRect.top())  / layer->scale()),
                    int(cropScene.width()  / layer->scale()),
                    int(cropScene.height() / layer->scale())
                    );

                cropImg = cropImg.intersected(layer->image().rect());

                if (!cropImg.isEmpty()) {

                    auto cmd = std::make_unique<CropCommand>(
                        *m_layerManager,
                        cropImg
                        );

                    emit commandReady(cmd.release());

                }
            }
        }

        setCropMode(false);
        m_dragContext = DragContext::None;
        event->accept();
        return;
    }


    if (m_dragContext == DragContext::MoveLayer)
    {
        auto layer = std::dynamic_pointer_cast<PixelLayer>(
            m_layerManager->layerAt(m_dragLayerIndex));

        if (layer) {
            QPointF finalOffset = layer->offset();

            if (finalOffset != m_initialOffset) {
                auto cmd = std::make_unique<MoveLayerCommand>(
                    *m_layerManager,
                    m_dragLayerIndex,
                    m_initialOffset,
                    finalOffset
                    );
                emit commandReady(cmd.release());
            }
        }
    }

    if (m_dragContext == DragContext::ScaleLayer)
    {
        auto layer = std::dynamic_pointer_cast<PixelLayer>(
            m_layerManager->layerAt(m_dragLayerIndex));

        if (layer) {
            QPointF finalOffset = layer->offset();
            float finalScale = layer->scale();

            if (finalScale != m_scaleStartScale || finalOffset != m_scaleStartOffset) {
                auto cmd = std::make_unique<TransformLayerCommand>(
                    *m_layerManager,
                    m_dragLayerIndex,
                    m_scaleStartOffset, m_scaleStartScale,
                    finalOffset, finalScale
                    );
                emit commandReady(cmd.release());
            }
        }
    }

    if (m_layerManager && event->button() == Qt::LeftButton)
        m_layerManager->setPainting(false);


    if (m_dragContext == DragContext::Paint && m_activeTool) {

        if (m_layerManager)
            m_layerManager->setPainting(false);

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

qreal MyGraphicsView::getHandleSize() const
{
    qreal zoom = transform().m11();
    return kHandleSize / zoom;
}

QVector<QRectF> MyGraphicsView::handleRects(const QRectF& bounds) const
{
    QVector<QRectF> rects;
    if (bounds.isNull())
        return rects;

    const qreal handleSize = getHandleSize();
    const qreal half = handleSize / 2.0;

    rects << QRectF(bounds.topLeft()     - QPointF(half, half), QSizeF(handleSize, handleSize))
          << QRectF(bounds.topRight()    - QPointF(half, half), QSizeF(handleSize, handleSize))
          << QRectF(bounds.bottomRight() - QPointF(half, half), QSizeF(handleSize, handleSize))
          << QRectF(bounds.bottomLeft()  - QPointF(half, half), QSizeF(handleSize, handleSize));

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

    qreal zoom = transform().m11();
    qreal penWidth = 1.0 / zoom;

    painter->setPen(QPen(Qt::white, penWidth));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(bounds);

    painter->setBrush(Qt::white);
    for (const auto& r : handleRects(bounds))
        painter->drawRect(r);

    painter->restore();
}

void MyGraphicsView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        if (m_dragContext == DragContext::Crop || getCropMode())
        {
            rubberBand->hide();
            setCropMode(false);
            m_dragContext = DragContext::None;
            unsetCursor();

            event->accept();
            return;
        }
    }

    QGraphicsView::keyPressEvent(event);
}

