#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QSlider>
#include "mygraphicsview.h"
#include <QTransform>
#include <QGraphicsItem>
#include <QRgb>
#include <QColor>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene{new QGraphicsScene(this)}
{


    ui->setupUi(this);


    ui->graphicsView->setScene(scene);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);



    connect(ui->scaleSlider, &QSlider::valueChanged, this, [this](int value) {
        double scale = static_cast<double>(value) / 100.0;
        QTransform t;
        t.scale(scale, scale);
        ui->graphicsView->setTransform(t);
    });

    connect(ui->graphicsView, &MyGraphicsView::zoomChanged, this, [this] (double scale) {
        ui->scaleSlider->setValue(static_cast<int>(scale * 100));
    });


    connect(ui->brightnessSlider, &QSlider::valueChanged, this, updateImage);
    connect(ui->saturationSlider, &QSlider::valueChanged, this, updateImage);
    connect(ui->contrastSlider, &QSlider::valueChanged, this, updateImage);
    connect(ui->blurSlider, &QSlider::valueChanged, this, updateImage);

    ui->bwButton->setCheckable(true);
    connect(ui->bwButton, &QPushButton::toggled, this, [this](bool checked) {
        imageState.blackWhiteFilter = checked;
        updateImage();
    });
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered() {
    QString fileName { QFileDialog::getOpenFileName(
        this,
        "Open Image",
        "",
            "Images (*.png *.jpg *.jpeg *.webp *.bmp)")};

    if (!fileName.isEmpty()) {
        scene->clear();
        ui->graphicsView->setPixmap(fileName);

        originalImage = ui->graphicsView->getPixmap().toImage();
    }
}

void MainWindow::on_actionCrop_triggered() {
    ui->graphicsView->setCropMode(true);
}

void MainWindow::on_actionRotateLeft_triggered() {
    imageState.rotateAngle -= 90;
    updateImage();
}

void MainWindow::on_actionRotateRight_triggered() {
    imageState.rotateAngle += 90;
    updateImage();
}

void MainWindow::on_actionFlipHorizontally_triggered() {
    imageState.flipH = !imageState.flipH;
    updateImage();
}

void MainWindow::on_actionFlipVertically_triggered() {
    imageState.flipV = !imageState.flipV;
    updateImage();
}



void MainWindow::updateImage() {
    if (ui->graphicsView->getPixmap().isNull()) return;

    QImage newImage = originalImage;

    if (imageState.rotateAngle) {
        QTransform t;
        t.rotate(imageState.rotateAngle);
        newImage = newImage.transformed(t);
    }

    if (imageState.flipH) {
        newImage.flip(Qt::Horizontal);
    }

    if (imageState.flipV) {
        newImage.flip(Qt::Vertical);
    }



    imageState.brightness = ui->brightnessSlider->value();
    imageState.saturation = ui->saturationSlider->value();
    imageState.contrast   = ui->contrastSlider->value();
    imageState.blur       = ui->blurSlider->value();

    double saturationFactor = static_cast<double>(imageState.saturation) / 100.0 + 1.0;
    double contrastFactor = (259.0 * (imageState.contrast + 255.0)) / (255.0 * (259.0 - imageState.contrast));
    double blurFactor = imageState.blur * 0.4;

    if (blurFactor < 0.001) {
        blurFactor = 0.001;
    }


    //blur

    int radius = ceil(3 * blurFactor);
    int kernelSize = 2 * radius + 1;

    std::vector<double> kernel(kernelSize);

    double sum{};

    for (int i{-radius}; i <= radius; i++) {
        double value {std::exp(-(i*i) / (2*blurFactor*blurFactor))};
        kernel[i+radius] = value;
        sum += value;
    }

    for (double& x : kernel) {
        x /= sum;
    }
    ///


    for (std::size_t y {0}; y < newImage.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(newImage.scanLine(static_cast<int>(y)));
        for (std::size_t x {0}; x < newImage.width(); ++x) {
            QColor color { QColor::fromRgb(row[static_cast<int>(x)]) };

            //gaussian blur usage(horizontal + vertical)


            double rSum{};
            double gSum{};
            double bSum{};


            for (int i{-radius}; i <= radius; i++) {
                int pixelIndex {qBound(0, static_cast<int>(x) + i, newImage.width()-1)};
                QColor pixelColor {QColor::fromRgb(row[pixelIndex])};

                rSum += pixelColor.red() * kernel[i+radius];
                gSum += pixelColor.green() * kernel[i+radius];
                bSum += pixelColor.blue() * kernel[i+radius];
            }


            int rB {std::clamp(rSum, 0.0, 255.0)};
            int gB {std::clamp(gSum, 0.0, 255.0)};
            int bB {std::clamp(bSum, 0.0, 255.0)};

            color = QColor::fromRgb(rB, gB, bB);


            // brightness
            int r = qBound(0, color.red() + imageState.brightness, 255);
            int g = qBound(0, color.green() + imageState.brightness ,255);
            int b = qBound(0, color.blue() + imageState.brightness ,255);
            color.setRgb(r, g, b);

            // saturation
            int h, s, l;
            color.getHsl(&h, &s, &l);
            s = qBound(0, static_cast<int>(s*saturationFactor) , 255);
            color.setHsl(h, s, l);

            //contrast
            int rC = static_cast<int>(qBound(0.0, (color.red() - 128) * contrastFactor + 128, 255.0));
            int gC =  static_cast<int>(qBound(0.0, (color.green() - 128) * contrastFactor + 128, 255.0));
            int bC = static_cast<int>(qBound(0.0, (color.blue() - 128) * contrastFactor + 128, 255.0));
            color.setRgb(rC, gC, bC);



            //blackWhiteFilter
            if (imageState.blackWhiteFilter) {
                int gray = qGray(color.rgb());
                color.setRgb(gray, gray, gray);
            }

            row[x] = color.rgb();
        }
    }



    //blur


    ui->graphicsView->setPixmap(QPixmap::fromImage(newImage));
}


