#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <QImage>
#include <QString>
#include <optional>

class QWidget;

class ImageIO
{
public:
    static std::optional<QImage> openImage(QWidget* parent);
    static bool saveImage(QWidget* parent, const QImage& image, QString& inOutPath);
    static bool saveImageAs(QWidget* parent, const QImage& image, QString& outPath);
};

#endif // IMAGEIO_H
