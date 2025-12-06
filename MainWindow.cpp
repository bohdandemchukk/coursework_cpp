#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include <QFileDialog>
#include <QSlider>
#include "mygraphicsview.h"
#include <QTransform>
#include <memory>

#include "rotatefilter.h"
#include "flipfilter.h"
#include "blurfilter.h"
#include "sharpenfilter.h"
#include "BrightnessFilter.h"
#include "contrastfilter.h"
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
#include "cropcommand.h"

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

    connect(ui->graphicsView, &MyGraphicsView::zoomChanged, this, [this](double scale) {
        ui->scaleSlider->setValue(static_cast<int>(scale * 100));
    });

    connect(ui->brightnessSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.brightness, ui->brightnessSlider->value());
        }
    });

    connect(ui->saturationSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.saturation, ui->saturationSlider->value());
        }
    });

    connect(ui->contrastSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.contrast, ui->contrastSlider->value());
        }
    });

    connect(ui->blurSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.blur, ui->blurSlider->value());
        }
    });


    connect(ui->sharpSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.sharpness, ui->sharpSlider->value());
        }
    });


    ui->bwButton->setCheckable(true);
    connect(ui->bwButton, &QPushButton::toggled, this, [this](bool checked) {
        if (!m_isUpdatingSlider) {
            changeFilterBool(&filterState.BWFilter, checked);
        }
    });


    connect(ui->temperatureSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.temperature, ui->temperatureSlider->value());
        }
    });


    connect(ui->exposureSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.exposure, ui->exposureSlider->value());
        }
    });


    connect(ui->gammaSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.gamma, ui->gammaSlider->value());
        }
    });


    connect(ui->tintSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.tint, ui->tintSlider->value());
        }
    });


    connect(ui->vibranceSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.vibrance, ui->vibranceSlider->value());
        }
    });


    connect(ui->shadowSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.shadow, ui->shadowSlider->value());
        }
    });


    connect(ui->highlightSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.highlight, ui->highlightSlider->value());
        }
    });


    connect(ui->claritySlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.clarity, ui->claritySlider->value());
        }
    });


    connect(ui->vignetteSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.vignette, ui->vignetteSlider->value());
        }
    });


    connect(ui->grainSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.grain, ui->grainSlider->value());
        }
    });


    connect(ui->splitToningSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.splitToning, ui->splitToningSlider->value());
        }
    });


    connect(ui->fadeSlider, &QSlider::sliderReleased, this, [this]() {
        if (!m_isUpdatingSlider) {
            changeFilterInt(&filterState.fade, ui->fadeSlider->value());
        }
    });

    connect(ui->graphicsView, &MyGraphicsView::cropFinished, this, &MainWindow::onCropFinished);

    updateUndoRedoButtons();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeFilterInt(int* target, int newValue) {
    int oldValue {*target};

    auto command {std::make_unique<ChangeFilterIntCommand>(
        target, oldValue, newValue,
        [this]() {
            this->rebuildPipeline();
        }
        )};
    undoRedoStack.push(std::move(command));
    updateUndoRedoButtons();
}

void MainWindow::changeFilterBool(bool* target, bool newValue) {
    bool oldValue {*target};

    auto command {std::make_unique<ChangeFilterBoolCommand>(
        target,
        oldValue,
        newValue,
        [this]() {
            this->rebuildPipeline();

        }
        )};

    undoRedoStack.push(std::move(command));
    updateUndoRedoButtons();
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
        undoRedoStack.clear();

        m_isUpdatingSlider = true;
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
        m_isUpdatingSlider = false;

        updateUndoRedoButtons();
    }
}

void MainWindow::on_actionCrop_triggered() {
    ui->graphicsView->setCropMode(true);
}

void MainWindow::onCropFinished(const QRect& rect) {
    auto cmd {std::make_unique<CropCommand>(
        &originalImage,
        rect,
        [this]() {
            this->updateImage();
        }
    )};

    undoRedoStack.push(std::move(cmd));
    updateUndoRedoButtons();
}

void MainWindow::on_actionRotateLeft_triggered() {
    changeFilterInt(&filterState.rotateAngle, filterState.rotateAngle - 90);
}

void MainWindow::on_actionRotateRight_triggered() {
    changeFilterInt(&filterState.rotateAngle, filterState.rotateAngle + 90);
}

void MainWindow::on_actionFlipHorizontally_triggered() {
    changeFilterBool(&filterState.flipH, !filterState.flipH);
}

void MainWindow::on_actionFlipVertically_triggered() {
    changeFilterBool(&filterState.flipV, !filterState.flipV);
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

void MainWindow::on_actionUndo_triggered() {
    m_isUpdatingSlider = true;
    undoRedoStack.undo();

    ui->brightnessSlider->setValue(filterState.brightness);
    ui->saturationSlider->setValue(filterState.saturation);
    ui->contrastSlider->setValue(filterState.contrast);
    ui->blurSlider->setValue(filterState.blur);
    ui->sharpSlider->setValue(filterState.sharpness);
    ui->temperatureSlider->setValue(filterState.temperature);
    ui->exposureSlider->setValue(filterState.exposure);
    ui->gammaSlider->setValue(filterState.gamma);
    ui->tintSlider->setValue(filterState.tint);
    ui->vibranceSlider->setValue(filterState.vibrance);
    ui->shadowSlider->setValue(filterState.shadow);
    ui->highlightSlider->setValue(filterState.highlight);
    ui->claritySlider->setValue(filterState.clarity);
    ui->vignetteSlider->setValue(filterState.vignette);
    ui->grainSlider->setValue(filterState.grain);
    ui->splitToningSlider->setValue(filterState.splitToning);
    ui->fadeSlider->setValue(filterState.fade);
    ui->bwButton->setChecked(filterState.BWFilter);

    m_isUpdatingSlider = false;
    updateUndoRedoButtons();
}

void MainWindow::on_actionRedo_triggered() {
    m_isUpdatingSlider = true;
    undoRedoStack.redo();

    ui->brightnessSlider->setValue(filterState.brightness);
    ui->saturationSlider->setValue(filterState.saturation);
    ui->contrastSlider->setValue(filterState.contrast);
    ui->blurSlider->setValue(filterState.blur);
    ui->sharpSlider->setValue(filterState.sharpness);
    ui->temperatureSlider->setValue(filterState.temperature);
    ui->exposureSlider->setValue(filterState.exposure);
    ui->gammaSlider->setValue(filterState.gamma);
    ui->tintSlider->setValue(filterState.tint);
    ui->vibranceSlider->setValue(filterState.vibrance);
    ui->shadowSlider->setValue(filterState.shadow);
    ui->highlightSlider->setValue(filterState.highlight);
    ui->claritySlider->setValue(filterState.clarity);
    ui->vignetteSlider->setValue(filterState.vignette);
    ui->grainSlider->setValue(filterState.grain);
    ui->splitToningSlider->setValue(filterState.splitToning);
    ui->fadeSlider->setValue(filterState.fade);
    ui->bwButton->setChecked(filterState.BWFilter);

    m_isUpdatingSlider = false;
    updateUndoRedoButtons();
}

void MainWindow::updateUndoRedoButtons() {
    ui->actionUndo->setEnabled(undoRedoStack.canUndo());
    ui->actionRedo->setEnabled(undoRedoStack.canRedo());
}

void MainWindow::updateImage() {
    if (ui->graphicsView->getPixmap().isNull()) return;

    QImage result {pipeline.process(originalImage)};
    ui->graphicsView->setPixmap(QPixmap::fromImage(result));
}
