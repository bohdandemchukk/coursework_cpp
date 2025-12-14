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
#include <QPainter>
#include <QVector>
#include <memory>
#include "command.h"
#include "layermanager.h"


enum class DragContext {
    None,
    Paint,
    MoveLayer,
    ScaleLayer,
    Pan,
    Crop
};



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

    void setLayerManager(LayerManager* manager) { m_layerManager = manager; }


    QPoint mapToImage(const QPoint& viewPos) const;
    QPoint mapToActiveLayerImage(const QPointF& scenePos) const;
    LayerManager* layerManager() const { return m_layerManager;}





signals:
    void zoomChanged(double scale);
    void cropFinished(QRect rect);
    void commandReady(Command* cmd);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;



private:

    enum class DragMode { None, Move, Scale };

    QRectF activeLayerBounds() const;
    QVector<QRectF> handleRects(const QRectF& bounds) const;
    int hitHandle(const QPointF& scenePos) const;
    std::shared_ptr<PixelLayer> hitTestLayers(const QPointF& scenePos, int& outIndex) const;

    QPixmap m_pixmap{};


    bool m_cropMode {false};
    QPoint m_cropStart {};

    bool m_panMode {false};
    QPoint m_lastPanPoint{};
    QRubberBand *rubberBand{nullptr};

    Tool* m_activeTool {nullptr};


    DragContext m_dragContext = DragContext::None;
    DragMode m_dragMode {DragMode::None};
    LayerManager* m_layerManager {nullptr};
    int m_dragHandle {-1};
    int m_dragLayerIndex {-1};
    QPointF m_dragStartScene {};
    QPointF m_initialOffset {};
    float m_initialScale {1.0f};
    QPointF m_initialCenter {};



};

#endif // MYGRAPHICSVIEW_H
