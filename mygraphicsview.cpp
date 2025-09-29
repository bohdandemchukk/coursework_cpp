#include "mygraphicsview.h"
#include <QRectF>


MyGraphicsView::MyGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , rubberBand {new QRubberBand(QRubberBand::Rectangle, this)}
{}

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

    if (!getCropMode()) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        setCropStart(event->pos());
        rubberBand->setGeometry(QRect(getCropStart(), QSize()));
        rubberBand->show();
    }
    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent *event) {

    if (!getCropMode()) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (rubberBand->isVisible()) {
        rubberBand->setGeometry(QRect(getCropStart(), event->pos()).normalized());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent *event) {

    if (!getCropMode()) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    if (getCropMode() && event->button() == Qt::LeftButton && rubberBand->isVisible()) {
        rubberBand->hide();

        QRectF sceneRect = mapToScene(rubberBand->geometry()).boundingRect();

        QPixmap cropped = getPixmap().copy(sceneRect.toRect());

        if (!cropped.isNull()) {
            setPixmap(cropped);
        }

        setCropMode(false);

    }

    QGraphicsView::mouseReleaseEvent(event);
}
