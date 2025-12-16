#include "imageio.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>

std::optional<QImage> ImageIO::openImage(QWidget* parent)
{
    const QString fileName = QFileDialog::getOpenFileName(
        parent,
        QObject::tr("Open Image"),
        QString(),
        QObject::tr("Images (*.png *.jpg *.jpeg *.webp *.bmp)")
        );

    if (fileName.isEmpty())
        return std::nullopt;

    QImage img(fileName);
    if (img.isNull()) {
        QMessageBox::warning(parent, "Error", "Failed to load image");
        return std::nullopt;
    }

    return img;
}

bool ImageIO::saveImage(QWidget* parent,
                        const QImage& image,
                        QString& inOutPath)
{
    if (image.isNull()) {
        QMessageBox::warning(parent, "Error", "No image to save");
        return false;
    }

    if (inOutPath.isEmpty())
        return saveImageAs(parent, image, inOutPath);

    if (!image.save(inOutPath)) {
        QMessageBox::warning(parent, "Error", "Failed to save image");
        return false;
    }

    return true;
}

bool ImageIO::saveImageAs(QWidget* parent,
                          const QImage& image,
                          QString& outPath)
{
    const QString fileName = QFileDialog::getSaveFileName(
        parent,
        QObject::tr("Save Image As"),
        outPath,
        QObject::tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)")
        );

    if (fileName.isEmpty())
        return false;

    if (!image.save(fileName)) {
        QMessageBox::warning(parent, "Error", "Failed to save image");
        return false;
    }

    outPath = fileName;
    return true;
}
