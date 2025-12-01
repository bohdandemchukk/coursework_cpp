#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include <QFileDialog>
#include <QSlider>
#include "mygraphicsview.h"
#include <QTransform>
#include <QGraphicsItem>
#include <QRgb>
#include <QColor>
#include <memory>

#include "rotatefilter.h"
#include "flipfilter.h"
#include "blurfilter.h"
#include "sharpenfilter.h"
#include "BrightnessFilter.h"
#include "contrastfilter.h"
#include "sharpenfilter.h"
#include "BWFilter.h"
#include "filterpipeline.h"
#include "saturationfilter.h"
#include "temperaturefilter.h"


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
    connect(ui->brightnessSlider, &QSlider::valueChanged, this, [this](int value) {
        filterState.brightness = value;
        rebuildPipeline();
    });
    connect(ui->saturationSlider, &QSlider::valueChanged, this, [this](int value) {
        filterState.saturation = value;
        rebuildPipeline();
    });

    connect(ui->contrastSlider, &QSlider::valueChanged, this, [this](int value) {
        filterState.contrast = value;
        rebuildPipeline();
    });
    connect(ui->blurSlider, &QSlider::valueChanged, this, [this](int value){
        filterState.blur = value;
        rebuildPipeline();
    });
    connect(ui->sharpSlider, &QSlider::valueChanged, this, [this](int value) {
        filterState.sharpness = value;
        rebuildPipeline();
    });

    ui->bwButton->setCheckable(true);
    connect(ui->bwButton, &QPushButton::toggled, this, [this](bool checked) {
        filterState.BWFilter = checked;
        rebuildPipeline();
    });

    connect(ui->temperatureSlider, QSlider::valueChanged, this, [this](int value) {
        filterState.temperature = value;
        rebuildPipeline();
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

        filterState = FilterState{};
        pipeline.clear();

        ui->brightnessSlider->setValue(0);
        ui->saturationSlider->setValue(0);
        ui->contrastSlider->setValue(0);
        ui->blurSlider->setValue(0);
        ui->sharpSlider->setValue(0);
        ui->bwButton->setChecked(false);
    }
}


///////////////////////////////////////
void MainWindow::on_actionCrop_triggered() {
    ui->graphicsView->setCropMode(true);
}
//////////////////////////////////////




void MainWindow::on_actionRotateLeft_triggered() {
    filterState.rotateAngle -= 90;
    rebuildPipeline();
}

void MainWindow::on_actionRotateRight_triggered() {
    filterState.rotateAngle += 90;
    rebuildPipeline();
}

void MainWindow::on_actionFlipHorizontally_triggered() {
    filterState.flipH = !filterState.flipH;
    rebuildPipeline();
}

void MainWindow::on_actionFlipVertically_triggered() {
    filterState.flipV = !filterState.flipV;
    rebuildPipeline();
}


void MainWindow::rebuildPipeline() {
    pipeline.clear();

    pipeline.addFilter(std::make_unique<RotateFilter>(filterState.rotateAngle));
    pipeline.addFilter(std::make_unique<FlipFilter>(FlipFilter::Direction::Horizontal, filterState.flipH));
    pipeline.addFilter(std::make_unique<FlipFilter>(FlipFilter::Direction::Vertical, filterState.flipV));

    pipeline.addFilter(std::make_unique<BlurFilter>(filterState.blur));
    pipeline.addFilter(std::make_unique<SharpenFilter>(filterState.sharpness));

    pipeline.addFilter(std::make_unique<BrightnessFilter>(filterState.brightness));
    pipeline.addFilter(std::make_unique<ContrastFilter>(filterState.contrast));
    pipeline.addFilter(std::make_unique<SaturationFilter>(filterState.saturation));
    pipeline.addFilter(std::make_unique<TemperatureFilter>(filterState.temperature));

    pipeline.addFilter(std::make_unique<BWFilter>(filterState.BWFilter));

    updateImage();
}




void MainWindow::updateImage() {
    if (ui->graphicsView->getPixmap().isNull()) return;

    QImage result {pipeline.process(originalImage)};

    ui->graphicsView->setPixmap(QPixmap::fromImage(result));
}


