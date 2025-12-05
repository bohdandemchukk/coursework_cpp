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
#include "exposurefilter.h"
#include "gammafilter.h"
#include "tintfilter.h"
#include "vibrancefilter.h"
#include "shadowfilter.h"
#include "highlightfilter.h"
#include "clarityFilter.h"
#include "vignettefilter.h"
#include "grainfilter.h"
#include "splittoningfilter.h"
#include "fadefilter.h"
#include "undoredo.h"


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
        applyFilterChange(filterState.brightness, value);
    });
    connect(ui->saturationSlider, &QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.saturation, value);
    });

    connect(ui->contrastSlider, &QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.contrast, value);
    });
    connect(ui->blurSlider, &QSlider::valueChanged, this, [this](int value){
        applyFilterChange(filterState.blur, value);
    });
    connect(ui->sharpSlider, &QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.sharpness, value);
    });

    ui->bwButton->setCheckable(true);
    connect(ui->bwButton, &QPushButton::toggled, this, [this](bool checked) {
        applyFilterChange(filterState.BWFilter, checked);
    });

    connect(ui->temperatureSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.temperature, value);
    });

    connect(ui->exposureSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.exposure, value);
    });

    connect(ui->gammaSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.gamma, value);
    });

    connect(ui->tintSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.tint, value);
    });

    connect(ui->vibranceSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.vibrance, value);
    });

    connect(ui->shadowSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.shadow, value);
    });

    connect(ui->highlightSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.highlight, value);
    });

    connect(ui->claritySlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.clarity, value);
    });

    connect(ui->vignetteSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.vignette, value);
    });

    connect(ui->grainSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.grain, value);
    });

    connect(ui->splitToningSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.splitToning, value);
    });

    connect(ui->fadeSlider, QSlider::valueChanged, this, [this](int value) {
        applyFilterChange(filterState.fade, value);
    });

    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::on_actionUndo_triggered);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::on_actionRedo_triggered);
    refreshUndoRedoActions();
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
        commandManager.clear();
        suppressCommands = true;

        ui->bwButton->setChecked(false);
        ui->brightnessSlider->setValue(0);
        ui->saturationSlider->setValue(0);
        ui->contrastSlider->setValue(0);
        ui->blurSlider->setValue(0);
        ui->sharpSlider->setValue(0);
        ui->temperatureSlider->setValue(0);
        ui->tintSlider->setValue(0);
        ui->vibranceSlider->setValue(0);
        ui->shadowSlider->setValue(0);
        ui->highlightSlider->setValue(0);
        ui->claritySlider->setValue(0);
        ui->vignetteSlider->setValue(0);
        ui->grainSlider->setValue(0);
        ui->splitToningSlider->setValue(0);
        ui->fadeSlider->setValue(0);
        suppressCommands = false;
        refreshUndoRedoActions();
    }
}


///////////////////////////////////////
void MainWindow::on_actionCrop_triggered() {
    ui->graphicsView->setCropMode(true);
}
//////////////////////////////////////




void MainWindow::on_actionRotateLeft_triggered() {
    applyFilterChange(filterState.rotateAngle, filterState.rotateAngle - 90);
}

void MainWindow::on_actionRotateRight_triggered() {
    applyFilterChange(filterState.rotateAngle, filterState.rotateAngle + 90);
}

void MainWindow::on_actionFlipHorizontally_triggered() {
    applyFilterChange(filterState.flipH, !filterState.flipH);
}

void MainWindow::on_actionFlipVertically_triggered() {
    applyFilterChange(filterState.flipV, !filterState.flipV);
}


void MainWindow::rebuildPipeline() {
    pipeline.clear();

    pipeline.addFilter(std::make_unique<RotateFilter>(filterState.rotateAngle));
    pipeline.addFilter(std::make_unique<FlipFilter>(FlipFilter::Direction::Horizontal, filterState.flipH));
    pipeline.addFilter(std::make_unique<FlipFilter>(FlipFilter::Direction::Vertical, filterState.flipV));

    pipeline.addFilter(std::make_unique<BlurFilter>(filterState.blur));
    pipeline.addFilter(std::make_unique<SharpenFilter>(filterState.sharpness));


    pipeline.addFilter(std::make_unique<ExposureFilter>(filterState.exposure));
    pipeline.addFilter(std::make_unique<ContrastFilter>(filterState.contrast));
    pipeline.addFilter(std::make_unique<BrightnessFilter>(filterState.brightness));
    pipeline.addFilter(std::make_unique<GammaFilter>(filterState.gamma));
    pipeline.addFilter(std::make_unique<ClarityFilter>(filterState.clarity));


    pipeline.addFilter(std::make_unique<TemperatureFilter>(filterState.temperature));
    pipeline.addFilter(std::make_unique<TintFilter>(filterState.tint));
    pipeline.addFilter(std::make_unique<SaturationFilter>(filterState.saturation));
    pipeline.addFilter(std::make_unique<VibranceFilter>(filterState.vibrance));
    pipeline.addFilter(std::make_unique<FadeFilter>(filterState.fade));

    pipeline.addFilter(std::make_unique<ShadowFilter>(filterState.shadow));
    pipeline.addFilter(std::make_unique<HighlightFilter>(filterState.highlight));
    pipeline.addFilter(std::make_unique<GrainFilter>(filterState.grain));
    pipeline.addFilter(std::make_unique<SplitToningFilter>(filterState.splitToning));

    pipeline.addFilter(std::make_unique<VignetteFilter>(filterState.vignette));

    pipeline.addFilter(std::make_unique<BWFilter>(filterState.BWFilter));




    updateImage();
}

void MainWindow::on_actionUndo_triggered()
{
    if (commandManager.undo()) {
        refreshUndoRedoActions();
    }
}

void MainWindow::on_actionRedo_triggered()
{
    if (commandManager.redo()) {
        refreshUndoRedoActions();
    }
}

void MainWindow::refreshUndoRedoActions()
{
    ui->actionUndo->setEnabled(commandManager.canUndo());
    ui->actionRedo->setEnabled(commandManager.canRedo());
}




void MainWindow::updateImage() {
    if (ui->graphicsView->getPixmap().isNull()) return;

    QImage result {pipeline.process(originalImage)};

    ui->graphicsView->setPixmap(QPixmap::fromImage(result));
}


