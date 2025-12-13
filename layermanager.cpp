#include "layermanager.h"

#include <QPainter>
#include <QPoint>
#include <memory>

namespace {
QPainter::CompositionMode toQtMode(BlendMode mode)
{
    switch (mode) {
    case BlendMode::Multiply: return QPainter::CompositionMode_Multiply;
    case BlendMode::Screen:   return QPainter::CompositionMode_Screen;
    case BlendMode::Overlay:  return QPainter::CompositionMode_Overlay;
    default:                  return QPainter::CompositionMode_SourceOver;
    }
}
}

LayerManager::LayerManager(const QSize &canvasSize, QImage::Format format)
    : m_canvasSize(canvasSize), m_format(format)
{
}

int LayerManager::layerCount() const
{
    return static_cast<int>(m_layers.size());
}

const std::vector<std::shared_ptr<Layer>> &LayerManager::layers() const
{
    return m_layers;
}

std::shared_ptr<Layer> LayerManager::layerAt(int index) const
{
    if (index < 0 || index >= layerCount())
        return nullptr;
    return m_layers[static_cast<size_t>(index)];
}

int LayerManager::addLayer(const std::shared_ptr<Layer> &layer, int index)
{
    if (!layer)
        return -1;

    if (index < 0 || index > layerCount())
        index = layerCount();

    m_layers.insert(m_layers.begin() + index, layer);
    clampActiveIndex();
    if (m_activeLayerIndex < 0)
        m_activeLayerIndex = index;

    notifyChanged();
    return index;
}

std::shared_ptr<Layer> LayerManager::removeLayer(int index)
{
    if (index < 0 || index >= layerCount())
        return nullptr;

    auto it = m_layers.begin() + index;
    auto removed = *it;
    m_layers.erase(it);
    clampActiveIndex();
    notifyChanged();
    return removed;
}

bool LayerManager::moveLayer(int from, int to)
{
    if (from < 0 || from >= layerCount() || to < 0 || to >= layerCount() || from == to)
        return false;

    auto layer = m_layers[static_cast<size_t>(from)];
    m_layers.erase(m_layers.begin() + from);
    m_layers.insert(m_layers.begin() + to, layer);
    if (m_activeLayerIndex == from)
        m_activeLayerIndex = to;
    notifyChanged();
    return true;
}

void LayerManager::setCanvasSize(const QSize &size)
{
    m_canvasSize = size;
    notifyChanged();
}

QSize LayerManager::canvasSize() const
{
    return m_canvasSize;
}

int LayerManager::activeLayerIndex() const
{
    return m_activeLayerIndex;
}

void LayerManager::setActiveLayerIndex(int index)
{
    if (index == m_activeLayerIndex || index < 0 || index >= layerCount())
        return;

    m_activeLayerIndex = index;
    notifyChanged();
}

std::shared_ptr<Layer> LayerManager::activeLayer()
{
    return layerAt(m_activeLayerIndex);
}

std::shared_ptr<const Layer> LayerManager::activeLayer() const
{
    return layerAt(m_activeLayerIndex);
}

void LayerManager::notifyLayerChanged()
{
    notifyChanged();
}

void LayerManager::setOnChanged(ChangeCallback callback)
{
    m_onChanged = std::move(callback);
}


QImage LayerManager::composite() const
{
    if (!m_canvasSize.isValid())
        return QImage();

    QImage result(m_canvasSize, m_format);
    result.fill(Qt::transparent);

    for (const auto &layer : m_layers)
    {
        if (!layer || !layer->isVisible())
            continue;


        if (layer->type() == LayerType::Pixel)
        {
            auto pixel = std::static_pointer_cast<PixelLayer>(layer);
            QPainter painter(&result);
            painter.setOpacity(pixel->opacity());
            painter.setCompositionMode(toQtMode(pixel->blendMode()));
            painter.drawImage(QPoint(0, 0), pixel->image());
        }
        else if (layer->type() == LayerType::Adjustment)
        {
            auto adjustment = std::static_pointer_cast<AdjustmentLayer>(layer);
            QImage filtered = adjustment->pipeline().process(result);

            QPainter painter(&result);
            painter.setOpacity(adjustment->opacity());
            painter.setCompositionMode(toQtMode(adjustment->blendMode()));
            painter.drawImage(QPoint(0, 0), filtered);
        }
    }

    return result;
}

void LayerManager::notifyChanged() const
{
    if (m_onChanged)
        m_onChanged();
}

void LayerManager::clampActiveIndex()
{
    if (m_layers.empty())
    {
        m_activeLayerIndex = -1;
        return;
    }

    if (m_activeLayerIndex >= layerCount())
        m_activeLayerIndex = layerCount() - 1;
}
