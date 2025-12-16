#ifndef LAYER_H
#define LAYER_H

#include <QImage>
#include <QString>
#include <QPointF>
#include <QRectF>
#include "pipeline/filterpipeline.h"

enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay
};

enum class LayerType {
    Pixel,
    Adjustment
};

class Layer
{
public:
    Layer(QString name, bool visible = true, float opacity = 1.0f);
    virtual ~Layer() = default;

    const QString& name() const;
    void setName(const QString& name);

    bool isVisible() const;
    void setVisible(bool visible);

    float opacity() const;
    void setOpacity(float opacity);

    BlendMode blendMode() const;
    void setBlendMode(BlendMode mode);

    bool isClipped() const { return m_clipped; }
    void setClipped(bool v) { m_clipped = v; }

    virtual QRectF bounds() const = 0;

    virtual LayerType type() const = 0;

private:
    QString   m_name;
    bool      m_visible;
    float     m_opacity;
    bool      m_clipped = false;
    BlendMode m_blendMode { BlendMode::Normal };
};


class PixelLayer final : public Layer
{
public:
    PixelLayer(QString name,
               const QImage& image,
               bool visible = true,
               float opacity = 1.0f);

    LayerType type() const override { return LayerType::Pixel; }

    const QImage& image() const;
    QImage& image();
    void setImage(const QImage& image);

    QPointF offset() const;
    void setOffset(const QPointF& p);

    float scale() const;
    void setScale(float s);

    QRectF bounds() const;

private:
    QImage  m_image;

    QPointF m_offset { 0.0, 0.0 };
    float   m_scale  { 1.0f };
};


class AdjustmentLayer : public Layer {
public:
    explicit AdjustmentLayer(QString name,
                             bool visible = true,
                             float opacity = 1.0f);

    LayerType type() const override;

    FilterPipeline& pipeline();
    const FilterPipeline& pipeline() const;
    const QImage& cachedProcess(const QImage& input) const;
    void markDirty();

    QRectF bounds() const;


private:
    mutable bool m_dirty = true;
    mutable QImage m_cached;

    FilterPipeline m_pipeline;
};



#endif // LAYER_H
