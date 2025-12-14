#include "layermanager.h"

#include <QPainter>
#include <QPoint>
#include <QDebug>
#include <algorithm>

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

LayerManager::LayerManager(const QSize& canvasSize, QImage::Format format)
    : m_canvasSize(canvasSize), m_format(format)
{
}

int LayerManager::layerCount() const
{
    return static_cast<int>(m_layers.size());
}

const std::vector<std::shared_ptr<Layer>>& LayerManager::layers() const
{
    return m_layers;
}

std::shared_ptr<Layer> LayerManager::layerAt(int index) const
{
    if (index < 0 || index >= layerCount())
        return nullptr;
    return m_layers[static_cast<size_t>(index)];
}

int LayerManager::addLayer(const std::shared_ptr<Layer>& layer, int index)
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

void LayerManager::setCanvasSize(const QSize& size)
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

void LayerManager::setPainting(bool painting)
{
    if (m_isPainting == painting)
        return;
    m_isPainting = painting;


    markDirty();
}

bool LayerManager::isPainting() const
{
    return m_isPainting;
}

static QColor applyBlendMode(const QColor& base, const QColor& blend, BlendMode mode)
{
    auto f = [](float v) { return std::clamp(v, 0.0f, 1.0f); };

    float bR = base.redF(),   bG = base.greenF(), bB = base.blueF();
    float aR = blend.redF(),  aG = blend.greenF(), aB = blend.blueF();

    float r, g, b;

    switch (mode) {
    case BlendMode::Multiply:
        r = bR * aR; g = bG * aG; b = bB * aB;
        break;
    case BlendMode::Screen:
        r = 1.0f - (1.0f - bR) * (1.0f - aR);
        g = 1.0f - (1.0f - bG) * (1.0f - aG);
        b = 1.0f - (1.0f - bB) * (1.0f - aB);
        break;
    case BlendMode::Overlay:
        r = (bR < 0.5f) ? (2.0f * bR * aR) : (1.0f - 2.0f * (1.0f - bR) * (1.0f - aR));
        g = (bG < 0.5f) ? (2.0f * bG * aG) : (1.0f - 2.0f * (1.0f - bG) * (1.0f - aG));
        b = (bB < 0.5f) ? (2.0f * bB * aB) : (1.0f - 2.0f * (1.0f - bB) * (1.0f - aB));
        break;
    default:
        r = aR; g = aG; b = aB;
        break;
    }

    return QColor::fromRgbF(f(r), f(g), f(b), base.alphaF());
}

static float lerp(float a, float b, float t) { return a + (b - a) * t; }

static QRgb blendPixel(QRgb base, QRgb adj, float opacity, BlendMode mode)
{
    float a = qAlpha(base) / 255.0f;
    if (a == 0.0f)
        return base;

    QColor cb(base);
    QColor ca(adj);
    QColor blended = applyBlendMode(cb, ca, mode);

    QColor out;
    out.setRedF(   lerp(cb.redF(),   blended.redF(),   opacity) );
    out.setGreenF( lerp(cb.greenF(), blended.greenF(), opacity) );
    out.setBlueF(  lerp(cb.blueF(),  blended.blueF(),  opacity) );
    out.setAlphaF(a);

    return out.rgba();
}

QImage LayerManager::composite() const
{
    if (!m_canvasSize.isValid())
        return QImage();

    if (!m_dirty && !m_cachedComposite.isNull())
        return m_cachedComposite;

    QImage result(m_canvasSize, m_format);
    result.fill(Qt::transparent);

    bool hasPending = false;
    QImage pendingImg;
    float pendingOpacity = 1.0f;
    BlendMode pendingBlend = BlendMode::Normal;
    QPointF pendingOffset {0.0, 0.0};
    float pendingScale = 1.0f;

    QPainter painter(&result);

    auto flushPending = [&]() {
        if (!hasPending) return;

        painter.setOpacity(pendingOpacity);
        painter.setCompositionMode(toQtMode(pendingBlend));
        painter.save();
        painter.translate(pendingOffset);
        painter.scale(pendingScale, pendingScale);
        painter.drawImage(QPoint(0, 0), pendingImg);
        painter.restore();

        hasPending = false;
        pendingImg = QImage();
    };

    const bool painting = m_isPainting;

    for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        const auto& layer = *it;
        if (!layer || !layer->isVisible())
            continue;

        if (layer->type() == LayerType::Pixel)
        {
            flushPending();

            auto pixel = std::static_pointer_cast<PixelLayer>(layer);

            bool willBeClipped = false;
            for (auto it2 = std::next(it); it2 != m_layers.end(); ++it2) {
                const auto& next = *it2;
                if (!next || !next->isVisible())
                    continue;

                if (next->type() == LayerType::Pixel)
                    break;

                if (next->type() == LayerType::Adjustment) {
                    auto a = std::static_pointer_cast<AdjustmentLayer>(next);
                    if (!a->isClipped())
                        break;
                    willBeClipped = true;
                }
            }


            const bool needCopy = (!painting && willBeClipped);
            pendingImg = needCopy ? pixel->image().copy()
                                  : pixel->image();

            pendingOpacity = pixel->opacity();
            pendingBlend   = pixel->blendMode();
            pendingOffset  = pixel->offset();
            pendingScale   = pixel->scale();
            hasPending = true;
        }
        else if (layer->type() == LayerType::Adjustment)
        {
            if (painting) {
                flushPending();
                continue;
            }

            auto adj = std::static_pointer_cast<AdjustmentLayer>(layer);

            if (adj->isClipped())
            {
                if (!hasPending) continue;

                const QImage& adjusted = adj->cachedProcess(pendingImg);

                for (int y = 0; y < pendingImg.height(); ++y) {
                    QRgb* b = reinterpret_cast<QRgb*>(pendingImg.scanLine(y));
                    const QRgb* a = reinterpret_cast<const QRgb*>(adjusted.constScanLine(y));
                    for (int x = 0; x < pendingImg.width(); ++x) {
                        if (qAlpha(b[x]) == 0) continue;
                        b[x] = blendPixel(b[x], a[x], adj->opacity(), adj->blendMode());
                    }
                }
            }
            else
            {
                flushPending();

                QImage adjusted = adj->pipeline().process(result);

                for (int y = 0; y < result.height(); ++y) {
                    QRgb* b = reinterpret_cast<QRgb*>(result.scanLine(y));
                    const QRgb* a = reinterpret_cast<const QRgb*>(adjusted.constScanLine(y));
                    for (int x = 0; x < result.width(); ++x) {
                        if (qAlpha(b[x]) == 0) continue;
                        b[x] = blendPixel(b[x], a[x], adj->opacity(), adj->blendMode());
                    }
                }
            }
        }
    }

    flushPending();

    m_cachedComposite = result;
    m_dirty = false;
    return m_cachedComposite;
}

void LayerManager::markDirty()
{
    m_dirty = true;
}

void LayerManager::notifyChanged()
{
    m_dirty = true;
    if (m_onChanged)
        m_onChanged();
}

void LayerManager::clampActiveIndex()
{
    if (m_layers.empty()) {
        m_activeLayerIndex = -1;
        return;
    }
    if (m_activeLayerIndex >= layerCount())
        m_activeLayerIndex = layerCount() - 1;
}
