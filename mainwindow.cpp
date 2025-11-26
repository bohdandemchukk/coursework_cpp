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
    connect(ui->sharpSlider, &QSlider::valueChanged, this, updateImage);

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
    imageState.sharpness  = ui->sharpSlider->value();

    double saturationFactor = static_cast<double>(imageState.saturation) / 100.0 + 1.0;
    double contrastFactor = (259.0 * (imageState.contrast + 255.0)) / (255.0 * (259.0 - imageState.contrast));


    if (imageState.sharpness > 0) {

        QImage savedOriginalImage {newImage};
        int sharpnessFactor {imageState.sharpness / 50};
        double blurFactor = 1.0;


        int radius = std::ceil(3 * blurFactor);
        int kernelSize = 2 * radius + 1;
        std::vector<double> kernel(kernelSize);
        double sum{};

        for (int i {-radius}; i <= radius; i++) {
            double value = std::exp(-(i*i)/(2*blurFactor*blurFactor));
            kernel[i+radius] = value;
            sum += value;
        }

        for (double& x : kernel) {
            x /= sum;
        }

        QImage temp = newImage;
        int width = temp.width();
        int height = temp.height();

        for (int y {0}; y < height; y++) {
            QRgb* src {reinterpret_cast<QRgb*>(temp.scanLine(y))};
            QRgb* dst {reinterpret_cast<QRgb*>(newImage.scanLine(y))};

            for (int x{0}; x < width; x++) {

                double rSum {};
                double gSum {};
                double bSum {};

                for (int i {-radius}; i <= radius; i++) {
                    int px = std::clamp(x+i, 0, width - 1);

                    rSum += qRed(src[px]) * kernel[i+radius];
                    gSum += qGreen(src[px]) * kernel[i+radius];
                    bSum += qBlue(src[px]) * kernel[i+radius];

                }

                dst[x] = qRgb(
                    static_cast<int>(std::clamp(rSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(gSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(bSum, 0.0, 255.0))
                    );
            }
        }

        temp = newImage;

        std::vector<int> rCol(height);
        std::vector<int> gCol(height);
        std::vector<int> bCol(height);

        for (int x {0}; x < width; x++) {


            for (int y{0}; y < height; y++) {
                QRgb px {reinterpret_cast<QRgb*>(temp.scanLine(y))[x]};
                rCol[y] = qRed(px);
                gCol[y] = qGreen(px);
                bCol[y] = qBlue(px);

                double rSum{};
                double gSum{};
                double bSum{};


                for (int j {-radius}; j <= radius; j++) {
                    int py = std::clamp(y+j, 0, height - 1);


                    rSum += rCol[py] * kernel[j+radius];
                    gSum += gCol[py] * kernel[j+radius];
                    bSum += bCol[py] * kernel[j+radius];
                }

                reinterpret_cast<QRgb*>(newImage.scanLine(y))[x] = qRgb(
                    static_cast<int>(std::clamp(rSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(gSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(bSum, 0.0, 255.0))
                    );
            }


        }

        for (int y {0}; y < height; y++) {
            QRgb* src {reinterpret_cast<QRgb*>(savedOriginalImage.scanLine(y))};
            QRgb* dst {reinterpret_cast<QRgb*>(newImage.scanLine(y))};

            for (int x{0}; x < width; x++) {

                QColor originalColor {QColor::fromRgb(src[x])};
                QColor blurredColor {QColor::fromRgb(dst[x])};

                int r = std::clamp((1+sharpnessFactor) * originalColor.red() - sharpnessFactor * blurredColor.red(), 0, 255);
                int g = std::clamp((1+sharpnessFactor) * originalColor.green() - sharpnessFactor * blurredColor.green(), 0, 255);
                int b = std::clamp((1+sharpnessFactor) * originalColor.blue() - sharpnessFactor * blurredColor.blue(), 0, 255);


                originalColor.setRgb(r, g, b);

                dst[x] = originalColor.rgb();
            }
        }


    }





    if (imageState.blur > 0)
    {
        double blurFactor = imageState.blur * 0.4;
        if (blurFactor < 0.001) blurFactor = 0.001;

        int radius = std::ceil(3 * blurFactor);
        int kernelSize = 2 * radius + 1;
        std::vector<double> kernel(kernelSize);
        double sum{};

        for (int i {-radius}; i <= radius; i++) {
            double value = std::exp(-(i*i)/(2*blurFactor*blurFactor));
            kernel[i+radius] = value;
            sum += value;
        }

        for (double& x : kernel) {
            x /= sum;
        }

        QImage temp = newImage;
        int width = temp.width();
        int height = temp.height();

        for (int y {0}; y < height; y++) {
            QRgb* src = reinterpret_cast<QRgb*>(temp.scanLine(y));
            QRgb* dst = reinterpret_cast<QRgb*>(newImage.scanLine(y));

            for (int x{0}; x < width; x++) {

                double rSum {};
                double gSum {};
                double bSum {};

                for (int i {-radius}; i <= radius; i++) {
                    int px = std::clamp(x+i, 0, width - 1);

                    rSum += qRed(src[px]) * kernel[i+radius];
                    gSum += qGreen(src[px]) * kernel[i+radius];
                    bSum += qBlue(src[px]) * kernel[i+radius];

                }

                dst[x] = qRgb(
                    static_cast<int>(std::clamp(rSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(gSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(bSum, 0.0, 255.0))
                    );
            }
        }

        temp = newImage;

        std::vector<int> rCol(height);
        std::vector<int> gCol(height);
        std::vector<int> bCol(height);

        for (int x {0}; x < width; x++) {


            for (int y{0}; y < height; y++) {
                QRgb px {reinterpret_cast<QRgb*>(temp.scanLine(y))[x]};
                rCol[y] = qRed(px);
                gCol[y] = qGreen(px);
                bCol[y] = qBlue(px);

                double rSum{};
                double gSum{};
                double bSum{};


                for (int j {-radius}; j <= radius; j++) {
                    int py = std::clamp(y+j, 0, height - 1);


                    rSum += rCol[py] * kernel[j+radius];
                    gSum += gCol[py] * kernel[j+radius];
                    bSum += bCol[py] * kernel[j+radius];
                }

                reinterpret_cast<QRgb*>(newImage.scanLine(y))[x] = qRgb(
                    static_cast<int>(std::clamp(rSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(gSum, 0.0, 255.0)),
                    static_cast<int>(std::clamp(bSum, 0.0, 255.0))
                    );
            }


        }
    }



    for (std::size_t y {0}; y < newImage.height(); ++y) {
        QRgb *row = reinterpret_cast<QRgb*>(newImage.scanLine(static_cast<int>(y)));

        for (std::size_t x {0}; x < newImage.width(); ++x) {
            QColor color { QColor::fromRgb(row[static_cast<int>(x)]) };

            // brightness
            int r = std::clamp(color.red() + imageState.brightness, 0, 255);
            int g = std::clamp(color.green() + imageState.brightness , 0, 255);
            int b = std::clamp(color.blue() + imageState.brightness, 0, 255);
            color.setRgb(r, g, b);

            // saturation
            int h, s, l;
            color.getHsl(&h, &s, &l);
            s = std::clamp(static_cast<int>(s*saturationFactor), 0, 255);
            color.setHsl(h, s, l);

            //contrast
            int rC = static_cast<int>(std::clamp((color.red() - 128) * contrastFactor + 128, 0.0, 255.0));
            int gC =  static_cast<int>(std::clamp((color.green() - 128) * contrastFactor + 128, 0.0, 255.0));
            int bC = static_cast<int>(std::clamp((color.blue() - 128) * contrastFactor + 128, 0.0, 255.0));
            color.setRgb(rC, gC, bC);



            //blackWhiteFilter
            if (imageState.blackWhiteFilter) {
                int gray = qGray(color.rgb());
                color.setRgb(gray, gray, gray);
            }

            row[x] = color.rgb();
        }
    }

    ui->graphicsView->setPixmap(QPixmap::fromImage(newImage));
}


