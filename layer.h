#ifndef LAYER_H
#define LAYER_H

#include <QImage>
#include <QString>
#include "filterpipeline.h"

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
    virtual bool isAdjustment() const {return false;}


    const QString &name() const;
    void setName(const QString &name);

    bool isVisible() const;
    void setVisible(bool visible);

    float opacity() const;
    void setOpacity(float opacity);

    BlendMode blendMode() const;
    void setBlendMode(BlendMode mode);

    virtual LayerType type() const = 0;

private:
    QString m_name;
    bool m_visible;
    float m_opacity;
    BlendMode m_blendMode {BlendMode::Normal};

};

class PixelLayer final : public Layer
{
public:
    PixelLayer(QString name, const QImage &image, bool visible = true, float opacity = 1.0f);

    LayerType type() const override { return LayerType::Pixel; }

    const QImage &image() const;
    QImage &image();
    void setImage(const QImage &image);

private:
    QImage m_image;
};

class AdjustmentLayer final : public Layer
{
public:
    AdjustmentLayer(QString name, bool visible = true, float opacity = 1.0f);
    bool isAdjustment() const override { return true; }
    LayerType type() const override { return LayerType::Adjustment; }

    FilterPipeline &pipeline() { return m_pipeline; }
    const FilterPipeline &pipeline() const { return m_pipeline; }

private:
    FilterPipeline m_pipeline;
};


#endif // LAYER_H
