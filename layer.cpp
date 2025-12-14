#include "layer.h"

#include <QtGlobal>
#include <algorithm>


Layer::Layer(QString name, bool visible, float opacity)
    : m_name(std::move(name)),
    m_visible(visible),
    m_opacity(opacity)
{}

const QString& Layer::name() const { return m_name; }
void Layer::setName(const QString& name) { m_name = name; }

bool Layer::isVisible() const { return m_visible; }
void Layer::setVisible(bool visible) { m_visible = visible; }

float Layer::opacity() const { return m_opacity; }
void Layer::setOpacity(float opacity) {
    m_opacity = qBound(0.0f, opacity, 1.0f);
}

BlendMode Layer::blendMode() const { return m_blendMode; }
void Layer::setBlendMode(BlendMode mode) { m_blendMode = mode; }


PixelLayer::PixelLayer(QString name,
                       const QImage& image,
                       bool visible,
                       float opacity)
    : Layer(std::move(name), visible, opacity),
    m_image(image)
{}

const QImage& PixelLayer::image() const { return m_image; }
QImage& PixelLayer::image() {
    Q_ASSERT(m_image.format() == QImage::Format_ARGB32_Premultiplied);
    return m_image;
}
void PixelLayer::setImage(const QImage& image) {
    if (image.format() == QImage::Format_ARGB32_Premultiplied)
        m_image = image;
    else
        m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

QPointF PixelLayer::offset() const {
    return m_offset;
}

void PixelLayer::setOffset(const QPointF& p) {
    m_offset = p;
}

float PixelLayer::scale() const {
    return m_scale;
}

void PixelLayer::setScale(float s) {
    m_scale = std::max(0.01f, s);
}

QRectF PixelLayer::bounds() const {
    QSizeF size = m_image.size();
    size *= m_scale;
    return QRectF(m_offset, size);
}


AdjustmentLayer::AdjustmentLayer(QString name,
                                 bool visible,
                                 float opacity)
    : Layer(std::move(name), visible, opacity)
{
}

LayerType AdjustmentLayer::type() const
{
    return LayerType::Adjustment;
}

FilterPipeline& AdjustmentLayer::pipeline()
{
    return m_pipeline;
}

const FilterPipeline& AdjustmentLayer::pipeline() const
{
    return m_pipeline;
}


void AdjustmentLayer::markDirty() { m_dirty = true; }

const QImage& AdjustmentLayer::cachedProcess(const QImage& input) const
{
    if (!m_dirty && !m_cached.isNull())
        return m_cached;

    m_cached = pipeline().process(input);
    m_dirty = false;
    return m_cached;
}


