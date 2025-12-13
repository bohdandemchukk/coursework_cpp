#include "layer.h"

#include <QtGlobal>

Layer::Layer(QString name, bool visible, float opacity)
    : m_name(std::move(name)), m_visible(visible), m_opacity(opacity)
{}

const QString &Layer::name() const { return m_name; }
void Layer::setName(const QString &name) { m_name = name; }

bool Layer::isVisible() const { return m_visible; }
void Layer::setVisible(bool visible) { m_visible = visible; }

float Layer::opacity() const { return m_opacity; }
void Layer::setOpacity(float opacity) { m_opacity = qBound(0.0f, opacity, 1.0f); }

BlendMode Layer::blendMode() const { return m_blendMode; }
void Layer::setBlendMode(BlendMode mode) { m_blendMode = mode; }

PixelLayer::PixelLayer(QString name, const QImage &image, bool visible, float opacity)
    : Layer(std::move(name), visible, opacity), m_image(image)
{}

const QImage &PixelLayer::image() const { return m_image; }
QImage &PixelLayer::image() { return m_image; }
void PixelLayer::setImage(const QImage &image) { m_image = image; }

AdjustmentLayer::AdjustmentLayer(QString name, bool visible, float opacity)
    : Layer(std::move(name), visible, opacity)
{}
