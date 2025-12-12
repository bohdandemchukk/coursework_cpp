#include "layer.h"

#include <QtGlobal>

Layer::Layer(QString name, const QImage &image, bool visible, float opacity)
    : m_name(std::move(name)), m_image(image), m_visible(visible), m_opacity(opacity)
{
}

const QString &Layer::name() const
{
    return m_name;
}

void Layer::setName(const QString &name)
{
    m_name = name;
}

const QImage &Layer::image() const
{
    return m_image;
}

QImage& Layer::image()
{
    return m_image;
}

void Layer::setImage(const QImage &image)
{
    m_image = image;
}

bool Layer::isVisible() const
{
    return m_visible;
}

void Layer::setVisible(bool visible)
{
    m_visible = visible;
}

float Layer::opacity() const
{
    return m_opacity;
}

void Layer::setOpacity(float opacity)
{
    m_opacity = qBound(0.0f, opacity, 1.0f);
}
