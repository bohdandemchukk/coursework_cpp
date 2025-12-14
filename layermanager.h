#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QImage>
#include <QSize>
#include <functional>
#include <memory>
#include <vector>
#include <QPainter>

#include "layer.h"

class LayerManager
{
public:
    using ChangeCallback = std::function<void()>;

    explicit LayerManager(const QSize &canvasSize = QSize(), QImage::Format format = QImage::Format_ARGB32_Premultiplied);

    int layerCount() const;
    const std::vector<std::shared_ptr<Layer>> &layers() const;
    std::shared_ptr<Layer> layerAt(int index) const;

    int addLayer(const std::shared_ptr<Layer> &layer, int index = -1);
    std::shared_ptr<Layer> removeLayer(int index);
    bool moveLayer(int from, int to);

    void setCanvasSize(const QSize &size);
    QSize canvasSize() const;

    std::shared_ptr<Layer> activeLayer();
    std::shared_ptr<const Layer> activeLayer() const;


    int activeLayerIndex() const;
    void setActiveLayerIndex(int index);

    void notifyLayerChanged();

    void setOnChanged(ChangeCallback callback);

    QImage composite() const;

    void markDirty();
    void notifyChanged();
    void setPainting(bool painting);
    bool isPainting() const;


private:
    mutable bool m_dirty = true;
    mutable QImage m_cachedComposite;
    bool m_isPainting = false;

    void clampActiveIndex();
    std::vector<std::shared_ptr<Layer>> m_layers;
    QSize m_canvasSize;
    QImage::Format m_format;
    int m_activeLayerIndex {-1};
    ChangeCallback m_onChanged {};
};

#endif
