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


class Layer
{
public:
    Layer(QString name, const QImage &image, bool visible = true, float opacity = 1.0f);

    const QString &name() const;
    void setName(const QString &name);

    const QImage &image() const;
    QImage& image();
    void setImage(const QImage &image);

    bool isVisible() const;
    void setVisible(bool visible);

    float opacity() const;
    void setOpacity(float opacity);

    BlendMode blendMode() const;
    void setBlendMode(BlendMode mode);

    FilterPipeline& pipeline() { return m_pipeline; }
    const FilterPipeline& pipeline() const { return m_pipeline; }

private:
    QString m_name;
    QImage m_image;
    bool m_visible;
    float m_opacity;
    BlendMode m_blendMode {BlendMode::Normal};
    FilterPipeline m_pipeline;
};

#endif // LAYER_H
