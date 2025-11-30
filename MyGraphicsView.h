#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QRubberBand>
#include <QPixmap>
#include <QGraphicsPixmapItem>


class MyGraphicsView: public QGraphicsView {
    Q_OBJECT

public:
    explicit MyGraphicsView(QWidget *parent = nullptr);

    QPixmap getPixmap() const {
        return m_pixmap;
    }

    QGraphicsPixmapItem *pixmapItem{};


    void setPixmap(const QPixmap &pixmap);


    QPoint getCropStart() const {
        return m_cropStart;
    }
    void setCropStart(QPoint cropStart) {
        m_cropStart = cropStart;
    }

    bool getCropMode() const {
        return m_cropMode;
    }

    void setCropMode(bool enabled) {
        m_cropMode = enabled;
    }



signals:
    void zoomChanged(double scale);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private:

    bool m_cropMode {false};

    QPixmap m_pixmap{};


    QPoint m_cropStart {};


    QRubberBand *rubberBand{nullptr};


};

#endif // MYGRAPHICSVIEW_H
