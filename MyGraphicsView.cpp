#include "MyGraphicsView.h"
#include <QRectF>
#include <QPointF>
#include <QScrollBar>
#include "tool.h"

MyGraphicsView::MyGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent),  rubberBand {new QRubberBand(QRubberBand::Rectangle, this)}
{
}

void MyGraphicsView::setPixmap(const QPixmap &pixmap) {
    m_pixmap = pixmap;

    auto *scene = new QGraphicsScene(this);
    pixmapItem = scene->addPixmap(pixmap);
    scene->setSceneRect(pixmap.rect());
    setScene(scene);
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

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event) {


    if (getCropMode() && event->button() == Qt::LeftButton && rubberBand->isVisible()) {
        rubberBand->hide();

        QRectF sceneRect = mapToScene(rubberBand->geometry()).boundingRect();

        QPixmap cropped = getPixmap().copy(sceneRect.toRect());

        if (!cropped.isNull()) {
            setPixmap(cropped);
        }

        setCropMode(false);

        emit cropFinished(sceneRect.toRect());

    }

    if (getPanMode() && event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
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
                auto cmd = m_activeTool->onMouseRelease(imagePoint, event->button());
                if (cmd) {
                    emit commandReady(std::move(cmd));
                }
                handled = true;
            }
        }

        if (!handled) {
            QGraphicsView::mouseReleaseEvent(event);
        }
        return;
    }


    QGraphicsView::mouseReleaseEvent(event);
}
