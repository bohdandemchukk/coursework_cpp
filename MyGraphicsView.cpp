#include "MyGraphicsView.h"
#include <QRectF>
#include <QPointF>
#include <QScrollBar>
#include "tool.h"

MyGraphicsView::MyGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent),  rubberBand {new QRubberBand(QRubberBand::Rectangle, this)}, pixmapItem{nullptr}
{
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


    if (!m_activeTool)
        return;

    QPoint imagePos = mapToImage(event->pos());
    m_activeTool->onMousePress(imagePos, event->button());

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
    }

    if (!getCropMode() && !getPanMode()) {
        bool handled = false;
        if (m_activeTool && pixmapItem) {
            QPointF scenePos = mapToScene(event->pos());
            QPoint imagePoint = pixmapItem->mapFromScene(scenePos).toPoint();
            if (pixmapItem->contains(scenePos)) {
                imagePoint.setX(qBound(0, imagePoint.x(), pixmapItem->pixmap().width() - 1));
                imagePoint.setY(qBound(0, imagePoint.y(), pixmapItem->pixmap().height() - 1));
                m_activeTool->onMousePress(imagePoint, event->button());
                handled = true;
            }
        }

        if (!handled) {
            QGraphicsView::mousePressEvent(event);
        }
        return;
    }



    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event) {

    if (!m_activeTool)
        return;

    QPoint imagePos = mapToImage(event->pos());
    m_activeTool->onMousePress(imagePos, event->button());

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

    if (!getCropMode() && !getPanMode()) {
        bool handled = false;
        if (m_activeTool && pixmapItem) {
            QPointF scenePos = mapToScene(event->pos());
            QPoint imagePoint = pixmapItem->mapFromScene(scenePos).toPoint();
            if (pixmapItem->contains(scenePos)) {
                imagePoint.setX(qBound(0, imagePoint.x(), pixmapItem->pixmap().width() - 1));
                imagePoint.setY(qBound(0, imagePoint.y(), pixmapItem->pixmap().height() - 1));
                m_activeTool->onMouseMove(imagePoint, event->buttons());
                handled = true;
            }
        }

        if (!handled) {
            QGraphicsView::mouseMoveEvent(event);
        }
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
    // --- PAN MODE ---
    if (getPanMode() && event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
        event->accept();
        return;
    }

    // --- CROP MODE ---
    if (getCropMode() && event->button() == Qt::LeftButton && rubberBand->isVisible()) {
        rubberBand->hide();

        QRectF sceneRect = mapToScene(rubberBand->geometry()).boundingRect();
        emit cropFinished(sceneRect.toRect());

        setCropMode(false);
        event->accept();
        return;
    }

    // --- TOOL MODE ---
    if (m_activeTool) {
        QPoint imagePos = mapToImage(event->pos());

        // 🔥 КЛІПИМО, АЛЕ НЕ RETURN
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



