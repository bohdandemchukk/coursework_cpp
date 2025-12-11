#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QRubberBand>
#include <QPixmap>
#include <QRect>
#include <QGraphicsPixmapItem>
#include <QtGlobal>
#include <memory>
#include "command.h"


class Tool;

class MyGraphicsView: public QGraphicsView {
    Q_OBJECT

public:
    explicit MyGraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr);




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

    void setPanMode(bool enabled) {
        m_panMode = enabled;
    }

    bool getPanMode() {
        return m_panMode;
    }

    void setActiveTool(Tool* tool) { m_activeTool = tool; }



signals:
    void zoomChanged(double scale);
    void cropFinished(QRect rect);
    void commandReady(std::unique_ptr<Command> command);
protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private:

    QPixmap m_pixmap{};

    bool m_cropMode {false};
    QPoint m_cropStart {};

    bool m_panMode {false};
    QPoint m_lastPanPoint{};
    QRubberBand *rubberBand{nullptr};

    Tool* m_activeTool {nullptr};


};

#endif // MYGRAPHICSVIEW_H
