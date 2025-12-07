#include "MyGraphicsView.h"
#include "MainWindow.h"
#include <QRectF>
#include <QPointF>
#include <QScrollBar>

QImage MainWindow::originalImage;

MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , rubberBand {new QRubberBand(QRubberBand::Rectangle, this)}
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
        QGraphicsView::mousePressEvent(event);
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
        QGraphicsView::mouseMoveEvent(event);
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
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }


    QGraphicsView::mouseReleaseEvent(event);
}
