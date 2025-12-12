#ifndef LAYER_H
#define LAYER_H

#include <QImage>
#include <QString>

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

private:
    QString m_name;
    QImage m_image;
    bool m_visible;
    float m_opacity; // 0.0 - 1.0
};

#endif // LAYER_H
