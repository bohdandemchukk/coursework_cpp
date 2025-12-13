#include "MyGraphicsView.h"

#include <QRectF>
#include <QPointF>
#include <QScrollBar>
#include <QPainter>
#include <algorithm>

#include "tool.h"
#include "layercommands.h"

namespace {
    constexpr qreal kHandleSize = 8.0;
}
MyGraphicsView::MyGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent),  rubberBand {new QRubberBand(QRubberBand::Rectangle, this)}, pixmapItem{nullptr}
{
    setMouseTracking(true);
}

void MyGraphicsView::setPixmap(const QPixmap &pixmap) {
    m_pixmap = pixmap;

    if (pixmapItem) {
        pixmapItem->setPixmap(pixmap);
        scene()->setSceneRect(pixmap.rect());
    } else {
        auto *scene = new QGraphicsScene(this);
        pixmapItem = scene->addPixmap(pixmap);
        scene->setSceneRect(pixmap.rect());
        setScene(scene);
    }
}

void MyGraphicsView::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0) {
        scale(1.1, 1.1);
    } else {
        scale(0.9, 0.9);
    }

    emit zoomChanged(transform().m11());
}

void MyGraphicsView::mousePressEvent(QMouseEvent *event) {

    QPointF scenePos = mapToScene(event->pos());

    if (getPanMode() && event->button() == Qt::LeftButton) {
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (getCropMode() && event->button() == Qt::LeftButton) {
        setCropStart(event->pos());
        rubberBand->setGeometry(QRect(getCropStart(), QSize()));
        rubberBand->show();
        return;
    }

    if (!getCropMode() && !getPanMode() && event->button() == Qt::LeftButton && m_layerManager) {
        int hitIndex = -1;
        auto pixelLayer = hitTestLayers(scenePos, hitIndex);
        if (pixelLayer) {
            m_layerManager->setActiveLayerIndex(hitIndex);
            m_dragStartScene = scenePos;
            m_initialOffset = pixelLayer->offset();
            m_initialScale = pixelLayer->scale();
            QSizeF imageSize = pixelLayer->image().size();
            m_initialCenter = m_initialOffset + QPointF(imageSize.width() * m_initialScale / 2.0,
                                                        imageSize.height() * m_initialScale / 2.0);

            int handleIndex = hitHandle(scenePos);
            if (handleIndex >= 0) {
                m_dragMode = DragMode::Scale;
                m_dragHandle = handleIndex;
                m_dragLayerIndex = hitIndex;
                event->accept();
                return;
            }

            if (pixelLayer->bounds().contains(scenePos)) {
                m_dragMode = DragMode::Move;
                m_dragLayerIndex = hitIndex;
                event->accept();
                return;
            }
        }
    }

    if (m_activeTool && !getCropMode() && !getPanMode()) {
        if (pixmapItem) {
            QPoint imagePoint = pixmapItem->mapFromScene(scenePos).toPoint();
            if (pixmapItem->contains(scenePos)) {
                imagePoint.setX(qBound(0, imagePoint.x(), pixmapItem->pixmap().width() - 1));
                imagePoint.setY(qBound(0, imagePoint.y(), pixmapItem->pixmap().height() - 1));
                m_activeTool->onMousePress(imagePoint, event->button());
                event->accept();
                return;
            }
        }

        m_activeTool->onMousePress(mapToImage(event->pos()), event->button());
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event) {

    QPointF scenePos = mapToScene(event->pos());

    if (getCropMode() && rubberBand->isVisible()) {
        rubberBand->setGeometry(QRect(getCropStart(), event->pos()).normalized());
    }

    if (getPanMode() && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = m_lastPanPoint - event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());

        m_lastPanPoint = event->pos();
        event->accept();
        return;
    }

    if (m_dragMode != DragMode::None && m_layerManager) {
        auto layer = std::dynamic_pointer_cast<PixelLayer>(m_layerManager->layerAt(m_dragLayerIndex));
        if (!layer) {
            m_dragMode = DragMode::None;
            return;
        }

        if (m_dragMode == DragMode::Move) {
            QPointF delta = scenePos - m_dragStartScene;
            layer->setOffset(m_initialOffset + delta);
            m_layerManager->notifyLayerChanged();
            event->accept();
            return;
        }

        if (m_dragMode == DragMode::Scale) {
            QPointF startVec = m_dragStartScene - m_initialCenter;
            QPointF currentVec = scenePos - m_initialCenter;
            if (startVec.manhattanLength() == 0)
                return;

            qreal factor = currentVec.manhattanLength() / startVec.manhattanLength();
            if (event->modifiers() & Qt::ShiftModifier) {
                QVector2D v0(startVec);
                QVector2D v1(currentVec);
                factor = v1.length() / v0.length();

            }

            float newScale = std::max(0.01f, static_cast<float>(m_initialScale * factor));
            QSizeF imageSize = layer->image().size();
            QPointF newOffset = m_initialCenter - QPointF(imageSize.width() * newScale / 2.0,
                                                          imageSize.height() * newScale / 2.0);

            layer->setScale(newScale);
            layer->setOffset(newOffset);
            m_layerManager->notifyLayerChanged();
            event->accept();
            return;
        }
    }

    if (m_activeTool && !getCropMode() && !getPanMode()) {
        if (pixmapItem) {
            QPoint imagePoint = pixmapItem->mapFromScene(scenePos).toPoint();
            if (pixmapItem->contains(scenePos)) {
                imagePoint.setX(qBound(0, imagePoint.x(), pixmapItem->pixmap().width() - 1));
                imagePoint.setY(qBound(0, imagePoint.y(), pixmapItem->pixmap().height() - 1));
                m_activeTool->onMouseMove(imagePoint, event->buttons());
                event->accept();
                return;
            }
        }

        m_activeTool->onMouseMove(mapToImage(event->pos()), event->buttons());
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

QPoint MyGraphicsView::mapToImage(const QPoint& viewPos) const {
    QPointF scenePos = mapToScene(viewPos);
    return scenePos.toPoint();
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (getPanMode() && event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }

    if (getCropMode() && event->button() == Qt::LeftButton && rubberBand->isVisible()) {
        rubberBand->hide();

        QRectF sceneRect = mapToScene(rubberBand->geometry()).boundingRect();
        emit cropFinished(sceneRect.toRect());

        setCropMode(false);
        event->accept();
        return;
    }

    if (m_dragMode != DragMode::None && m_layerManager) {
        auto layer = std::dynamic_pointer_cast<PixelLayer>(m_layerManager->layerAt(m_dragLayerIndex));
        if (layer) {
            std::unique_ptr<Command> cmd;
            if (m_dragMode == DragMode::Move) {
                if (layer->offset() != m_initialOffset) {
                    cmd = std::make_unique<MoveLayerCommand>(*m_layerManager, m_dragLayerIndex, m_initialOffset, layer->offset());
                }
            } else if (m_dragMode == DragMode::Scale) {
                if (!qFuzzyCompare(layer->scale(), m_initialScale)) {
                    cmd = std::make_unique<ScaleLayerCommand>(*m_layerManager, m_dragLayerIndex, m_initialScale, layer->scale());
                }
            }

            if (cmd) {
                emit commandReady(cmd.release());
            }
        }

        m_dragMode = DragMode::None;
        m_dragHandle = -1;
        m_dragLayerIndex = -1;
        event->accept();
        return;
    }

    if (m_activeTool) {
        QPoint imagePos = mapToImage(event->pos());

        if (pixmapItem) {
            imagePos.setX(qBound(0, imagePos.x(), pixmapItem->pixmap().width()  - 1));
            imagePos.setY(qBound(0, imagePos.y(), pixmapItem->pixmap().height() - 1));
        }

        std::unique_ptr<Command> cmd =
            m_activeTool->onMouseRelease(imagePos, event->button());

        if (cmd) {
            emit commandReady(cmd.release());
        }

        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

QRectF MyGraphicsView::activeLayerBounds() const
{
    if (!m_layerManager)
        return {};
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_layerManager->activeLayer());
    if (!layer)
        return {};
    return layer->bounds();
}

QVector<QRectF> MyGraphicsView::handleRects(const QRectF& bounds) const
{
    QVector<QRectF> rects;
    if (bounds.isNull())
        return rects;

    const qreal half = kHandleSize / 2.0;
    const QPointF corners[] = {
        bounds.topLeft(),
        bounds.topRight(),
        bounds.bottomRight(),
        bounds.bottomLeft()
    };

    for (const auto& corner : corners) {
        rects.append(QRectF(corner.x() - half, corner.y() - half, kHandleSize, kHandleSize));
    }
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

std::shared_ptr<PixelLayer> MyGraphicsView::hitTestLayers(const QPointF& scenePos, int& outIndex) const
{
    outIndex = -1;
    if (!m_layerManager)
        return nullptr;

    for (int i = m_layerManager->layerCount() - 1; i >= 0; --i) {
        auto layer = m_layerManager->layerAt(i);
        auto pixel = std::dynamic_pointer_cast<PixelLayer>(layer);
        if (!pixel || !layer->isVisible())
            continue;

        if (pixel->bounds().contains(scenePos)) {
            outIndex = i;
            return pixel;
        }
    }
    return nullptr;
}

void MyGraphicsView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    auto bounds = activeLayerBounds();
    if (bounds.isNull())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::white, 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(bounds);

    painter->setBrush(Qt::white);
    for (const auto& handle : handleRects(bounds)) {
        painter->drawRect(handle);
    }

    painter->restore();
}

