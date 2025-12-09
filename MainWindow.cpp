#include "MainWindow.h"

#include <QApplication>
#include <QFileDialog>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QSlider>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QFileInfo>
#include <QEasingCurve>
#include <algorithm>
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

AccordionSection::AccordionSection(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    m_headerButton = new QPushButton(title, this);
    m_headerButton->setCheckable(true);
    m_headerButton->setChecked(false);
    m_headerButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #2c2c2c;"
        "  color: #e0e0e0;"
        "  text-align: left;"
        "  padding: 8px 12px;"
        "  border: none;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #3a3a3a; }"
        "QPushButton:checked { background-color: #3f51b5; color: white; }"
    );

    m_content = new QWidget(this);
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(12, 4, 12, 12);
    m_contentLayout->setSpacing(10);
    m_content->setLayout(m_contentLayout);
    m_content->setMaximumHeight(0);
    m_content->setVisible(false);

    m_animation = new QPropertyAnimation(m_content, "maximumHeight", this);
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_headerButton);
    layout->addWidget(m_content);

    connect(m_headerButton, &QPushButton::clicked, this, &AccordionSection::toggle);
}

void AccordionSection::animateContent(int start, int end)
{
    m_animation->stop();
    m_animation->setStartValue(start);
    m_animation->setEndValue(end);
    m_animation->start();
}

void AccordionSection::toggle()
{
    setExpanded(!m_expanded);
    emit expanded(this);
}

void AccordionSection::setExpanded(bool expanded)
{
    if (expanded == m_expanded)
        return;

    m_expanded = expanded;
    m_headerButton->setChecked(expanded);

    int startHeight = m_content->maximumHeight();
    int endHeight = expanded ? m_content->sizeHint().height() : 0;

    m_content->setVisible(true);
    animateContent(startHeight, endHeight);

    if (!expanded) {
        connect(m_animation, &QPropertyAnimation::finished, m_content, [this]() {
            if (!m_expanded) {
                m_content->setVisible(false);
            }
        });
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , scene{new QGraphicsScene(this)}
{
    applyDarkPalette();
    setupUi();
    setupShortcuts();

    updateUndoRedoButtons();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyDarkPalette()
{
    setStyleSheet(
        "QMainWindow { background-color: #202020; }"
        "QMenuBar { background-color: #2a2a2a; color: #e0e0e0; }"
        "QMenuBar::item:selected { background: #3a3a3a; }"
        "QMenu { background-color: #2a2a2a; color: #e0e0e0; }"
        "QMenu::item:selected { background: #3f51b5; }"
        "QToolBar { background: #2c2c2c; spacing: 6px; padding: 4px; }"
        "QStatusBar { background: #2a2a2a; color: #cfcfcf; }"
        "QDockWidget { background: #252525; color: #e0e0e0; titlebar-close-icon: none; titlebar-normal-icon: none; }"
        "QLabel { color: #e0e0e0; }"
        "QSlider::groove:horizontal { height: 6px; background: #3a3a3a; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #90caf9; width: 16px; margin: -5px 0; border-radius: 8px; }"
        "QSlider::add-page:horizontal { background: #444; }"
        "QSlider::sub-page:horizontal { background: #3f51b5; }"
        "QPushButton#iconButton { background: transparent; border: none; color: #e0e0e0; padding: 6px; }"
        "QPushButton#iconButton:hover { background: #3a3a3a; border-radius: 4px; }"
    );
}

void MainWindow::setupUi()
{
    createMenus();
    createToolbar();

    QWidget *central = createCentralView();
    setCentralWidget(central);

    createFilterDock();

    statusBar()->showMessage("Ready");
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    openAction = fileMenu->addAction(tr("Open"), this, &MainWindow::openImage, QKeySequence::Open);
    saveAction = fileMenu->addAction(tr("Save"), this, &MainWindow::saveImage, QKeySequence::Save);

    QMenu *editMenu = menuBar()->addMenu(tr("Edit"));
    undoAction = editMenu->addAction(tr("Undo"), this, &MainWindow::undo, QKeySequence::Undo);
    redoAction = editMenu->addAction(tr("Redo"), this, &MainWindow::redo, QKeySequence::Redo);
    cropAction = editMenu->addAction(tr("Crop"), this, [this]() { graphicsView->setCropMode(true); });

    QMenu *imageMenu = menuBar()->addMenu(tr("Image"));
    rotateLeftAction = imageMenu->addAction(tr("Rotate Left"), this, &MainWindow::rotateLeft, QKeySequence("Ctrl+["));
    rotateRightAction = imageMenu->addAction(tr("Rotate Right"), this, &MainWindow::rotateRight, QKeySequence("Ctrl+]"));
    flipHAction = imageMenu->addAction(tr("Flip Horizontally"), this, &MainWindow::flipHorizontally, QKeySequence("Ctrl+H"));
    flipVAction = imageMenu->addAction(tr("Flip Vertically"), this, &MainWindow::flipVertically, QKeySequence("Ctrl+Shift+H"));

    QMenu *viewMenu = menuBar()->addMenu(tr("View"));
    fitToScreenAction = viewMenu->addAction(tr("Fit to Screen"), this, &MainWindow::fitToScreen, QKeySequence("Ctrl+0"));
    panToolAction = viewMenu->addAction(tr("Hand Tool"), this, &MainWindow::togglePanTool, QKeySequence("H"));
    panToolAction->setCheckable(true);
}

void MainWindow::createToolbar()
{
    QToolBar *toolbar = addToolBar(tr("Main Toolbar"));
    toolbar->setIconSize(QSize(22, 22));

    auto createIconButton = [&](const QString &text, const QIcon &icon, QAction *action) {
        if (!action) return;
        action->setIcon(icon);
        toolbar->addAction(action);
    };

    createIconButton("Open", style()->standardIcon(QStyle::SP_DialogOpenButton), openAction);
    createIconButton("Save", style()->standardIcon(QStyle::SP_DialogSaveButton), saveAction);
    toolbar->addSeparator();
    createIconButton("Undo", style()->standardIcon(QStyle::SP_ArrowBack), undoAction);
    createIconButton("Redo", style()->standardIcon(QStyle::SP_ArrowForward), redoAction);
    toolbar->addSeparator();

    brushAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DriveDVDIcon), tr("Brush"));
    brushAction->setCheckable(true);

    eraserAction = toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserStop), tr("Eraser"));
    eraserAction->setCheckable(true);

    panToolAction->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
    toolbar->addAction(panToolAction);

    toolbar->addSeparator();
    createIconButton("Hand", style()->standardIcon(QStyle::SP_ArrowUp), cropAction);
    createIconButton("Rotate L", style()->standardIcon(QStyle::SP_ArrowBack), rotateLeftAction);
    createIconButton("Rotate R", style()->standardIcon(QStyle::SP_ArrowForward), rotateRightAction);
    createIconButton("Flip H", style()->standardIcon(QStyle::SP_BrowserReload), flipHAction);
    createIconButton("Flip V", style()->standardIcon(QStyle::SP_ArrowDown), flipVAction);

    zoomLabel = new QLabel(tr("100%"), this);
    zoomLabel->setMargin(8);
    toolbar->addWidget(zoomLabel);
}

QWidget *MainWindow::createCentralView()
{
    QWidget *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    graphicsView = new MyGraphicsView(this);
    graphicsView->setScene(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    connect(graphicsView, &MyGraphicsView::zoomChanged, this, &MainWindow::updateZoomDisplay);
    connect(graphicsView, &MyGraphicsView::cropFinished, this, &MainWindow::onCropFinished);

    layout->addWidget(graphicsView);

    QSlider *zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(10, 300);
    zoomSlider->setValue(100);
    zoomSlider->setToolTip(tr("Zoom"));
    connect(zoomSlider, &QSlider::valueChanged, this, [this](int value) {
        double scale = static_cast<double>(value) / 100.0;
        QTransform t;
        t.scale(scale, scale);
        graphicsView->setTransform(t);
        updateZoomDisplay(scale);
    });
    layout->addWidget(zoomSlider);

    return central;
}

void MainWindow::createFilterDock()
{
    filterDock = new QDockWidget(tr("Filters"), this);
    filterDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    filterDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

    QWidget *dockContent = new QWidget(filterDock);
    auto *dockLayout = new QVBoxLayout(dockContent);
    dockLayout->setContentsMargins(8, 8, 8, 8);
    dockLayout->setSpacing(6);

    QList<AccordionSection*> sections;

    auto *basic = new AccordionSection(tr("Basic"), dockContent);
    m_sliders["Exposure"]   = addSlider(basic, tr("Exposure"), -100, 100, 0);
    m_sliders["Contrast"]   = addSlider(basic, tr("Contrast"), -100, 100, 0);
    m_sliders["Brightness"] = addSlider(basic, tr("Brightness"), -100, 100, 0);
    m_sliders["Highlights"] = addSlider(basic, tr("Highlights"), -100, 100, 0);
    m_sliders["Shadows"]    = addSlider(basic, tr("Shadows"), -100, 100, 0);
    m_sliders["Clarity"]    = addSlider(basic, tr("Clarity"), -100, 100, 0);
    sections << basic;

    auto *color = new AccordionSection(tr("Color"), dockContent);
    m_sliders["Temperature"] = addSlider(color, tr("Temperature"), -100, 100, 0);
    m_sliders["Tint"]        = addSlider(color, tr("Tint"), -100, 100, 0);
    m_sliders["Saturation"]  = addSlider(color, tr("Saturation"), -100, 100, 0);
    m_sliders["Vibrance"]    = addSlider(color, tr("Vibrance"), -100, 100, 0);
    sections << color;

    auto *effects = new AccordionSection(tr("Effects"), dockContent);
    m_sliders["SplitToning"] = addSlider(effects, tr("Split Toning"), -100, 100, 0);
    m_sliders["Vignette"]    = addSlider(effects, tr("Vignette"), -100, 100, 0);
    m_sliders["Grain"]       = addSlider(effects, tr("Grain"), 0, 100, 0);
    m_sliders["Fade"]        = addSlider(effects, tr("Fade"), -100, 100, 0);
    sections << effects;

    auto *detail = new AccordionSection(tr("Detail"), dockContent);
    m_sliders["Sharpen"] = addSlider(detail, tr("Sharpen"), 0, 100, 0);
    m_sliders["Blur"]    = addSlider(detail, tr("Blur"), 0, 100, 0);
    sections << detail;

    auto *transform = new AccordionSection(tr("Transform"), dockContent);
    QHBoxLayout *rotateLayout = new QHBoxLayout();
    QPushButton *rotateLeftButton = new QPushButton(tr("Rotate -90"));
    QPushButton *rotateRightButton = new QPushButton(tr("Rotate +90"));
    rotateLeftButton->setObjectName("iconButton");
    rotateRightButton->setObjectName("iconButton");
    rotateLeftButton->setStyleSheet("QPushButton { color: #e0e0e0; }");
    rotateRightButton->setStyleSheet("QPushButton { color: #e0e0e0; }");

    rotateLayout->addWidget(rotateLeftButton);
    rotateLayout->addWidget(rotateRightButton);
    transform->contentLayout()->addLayout(rotateLayout);

    auto *flipLayout = new QHBoxLayout();
    QPushButton *flipHButton = new QPushButton(tr("Flip H"));
    QPushButton *flipVButton = new QPushButton(tr("Flip V"));
    flipLayout->addWidget(flipHButton);
    flipLayout->addWidget(flipVButton);
    transform->contentLayout()->addLayout(flipLayout);

    bwButton = new QPushButton(tr("Black & White"));
    bwButton->setCheckable(true);
    bwButton->setStyleSheet("QPushButton { background: #3a3a3a; color: #e0e0e0; padding: 6px; border-radius: 4px; }");
    transform->contentLayout()->addWidget(bwButton);

    m_sliders["Gamma"] = addSlider(transform, tr("Gamma"), -100, 100, 0);
    sections << transform;

    for (AccordionSection *section : sections) {
        dockLayout->addWidget(section);
        connect(section, &AccordionSection::expanded, this, [sections, section](AccordionSection *active) {
            for (AccordionSection *other : sections) {
                if (other != active) {
                    other->setExpanded(false);
                }
            }
            section->setExpanded(true);
        });
    }

    if (!sections.isEmpty()) {
        sections.first()->setExpanded(true);
    }

    dockLayout->addStretch();
    dockContent->setLayout(dockLayout);
    filterDock->setWidget(dockContent);

    addDockWidget(Qt::LeftDockWidgetArea, filterDock);

    // Connections to filter pipeline
    connect(m_sliders["Exposure"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.exposure, m_sliders["Exposure"]->value()); });
    connect(m_sliders["Contrast"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.contrast, m_sliders["Contrast"]->value()); });
    connect(m_sliders["Brightness"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.brightness, m_sliders["Brightness"]->value()); });
    connect(m_sliders["Highlights"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.highlight, m_sliders["Highlights"]->value()); });
    connect(m_sliders["Shadows"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.shadow, m_sliders["Shadows"]->value()); });
    connect(m_sliders["Clarity"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.clarity, m_sliders["Clarity"]->value()); });

    connect(m_sliders["Temperature"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.temperature, m_sliders["Temperature"]->value()); });
    connect(m_sliders["Tint"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.tint, m_sliders["Tint"]->value()); });
    connect(m_sliders["Saturation"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.saturation, m_sliders["Saturation"]->value()); });
    connect(m_sliders["Vibrance"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.vibrance, m_sliders["Vibrance"]->value()); });

    connect(m_sliders["SplitToning"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.splitToning, m_sliders["SplitToning"]->value()); });
    connect(m_sliders["Vignette"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.vignette, m_sliders["Vignette"]->value()); });
    connect(m_sliders["Grain"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.grain, m_sliders["Grain"]->value()); });
    connect(m_sliders["Fade"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.fade, m_sliders["Fade"]->value()); });

    connect(m_sliders["Sharpen"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.sharpness, m_sliders["Sharpen"]->value()); });
    connect(m_sliders["Blur"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.blur, m_sliders["Blur"]->value()); });

    connect(m_sliders["Gamma"], &QSlider::sliderReleased, this, [this]() { changeFilterInt(&filterState.gamma, m_sliders["Gamma"]->value()); });

    connect(rotateLeftButton, &QPushButton::clicked, this, &MainWindow::rotateLeft);
    connect(rotateRightButton, &QPushButton::clicked, this, &MainWindow::rotateRight);
    connect(flipHButton, &QPushButton::clicked, this, &MainWindow::flipHorizontally);
    connect(flipVButton, &QPushButton::clicked, this, &MainWindow::flipVertically);

    connect(bwButton, &QPushButton::toggled, this, [this](bool checked) { changeFilterBool(&filterState.BWFilter, checked); });
}

QSlider *MainWindow::addSlider(AccordionSection *section, const QString &name, int min, int max, int defaultValue)
{
    QWidget *row = new QWidget(section);
    auto *rowLayout = new QVBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(4);

    QLabel *label = new QLabel(name, row);
    QSlider *slider = new QSlider(Qt::Horizontal, row);
    slider->setRange(min, max);
    slider->setValue(defaultValue);
    slider->setSingleStep(1);

    QLabel *valueLabel = new QLabel(QString::number(defaultValue), row);
    valueLabel->setAlignment(Qt::AlignRight);
    valueLabel->setStyleSheet("color: #9e9e9e; font-size: 11px;");

    connect(slider, &QSlider::valueChanged, valueLabel, [valueLabel](int v) {
        valueLabel->setText(QString::number(v));
    });

    rowLayout->addWidget(label);
    rowLayout->addWidget(slider);
    rowLayout->addWidget(valueLabel);

    section->contentLayout()->addWidget(row);
    return slider;
}

void MainWindow::openImage()
{
    QString fileName { QFileDialog::getOpenFileName(
        this,
        tr("Open Image"),
        "",
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp)"))};

    if (!fileName.isEmpty()) {
        scene->clear();
        graphicsView->setPixmap(QPixmap(fileName));
        originalImage = graphicsView->getPixmap().toImage();

        filterState = FilterState{};
        pipeline.clear();
        undoRedoStack.clear();

        m_isUpdatingSlider = true;
        resetFilterControls();
        m_isUpdatingSlider = false;

        updateUndoRedoButtons();
        statusBar()->showMessage(tr("Loaded %1").arg(QFileInfo(fileName).fileName()), 2000);
    }
}

void MainWindow::saveImage()
{
    if (graphicsView->getPixmap().isNull()) {
        QMessageBox::warning(this, tr("No image"), tr("Please open an image first."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "edited.png", tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (fileName.isEmpty()) return;

    graphicsView->getPixmap().save(fileName);
    statusBar()->showMessage(tr("Saved %1").arg(QFileInfo(fileName).fileName()), 2000);
}

void MainWindow::resetFilterControls()
{
    for (auto it = m_sliders.begin(); it != m_sliders.end(); ++it) {
        it.value()->setValue(0);
    }
    bwButton->setChecked(false);
}

void MainWindow::rotateLeft()
{
    changeFilterInt(&filterState.rotateAngle, filterState.rotateAngle - 90);
}

void MainWindow::rotateRight()
{
    changeFilterInt(&filterState.rotateAngle, filterState.rotateAngle + 90);
}

void MainWindow::flipHorizontally()
{
    changeFilterBool(&filterState.flipH, !filterState.flipH);
}

void MainWindow::flipVertically()
{
    changeFilterBool(&filterState.flipV, !filterState.flipV);
}

void MainWindow::fitToScreen()
{
    if (originalImage.isNull()) return;

    QSize imgSize {originalImage.size()};
    QSize viewSize {graphicsView->viewport()->size()};

    double scaleX {static_cast<double>(viewSize.width()) / static_cast<double>(imgSize.width())};
    double scaleY {static_cast<double>(viewSize.height()) / static_cast<double>(imgSize.height())};

    double scale {std::min(scaleX, scaleY)};
    updateZoomDisplay(scale);

    QTransform t;
    t.scale(scale, scale);
    graphicsView->setTransform(t);
}

void MainWindow::togglePanTool()
{
    m_isPanToolActive = !m_isPanToolActive;
    updatePanMode();
}

void MainWindow::undo()
{
    m_isUpdatingSlider = true;
    undoRedoStack.undo();

    m_sliders["Brightness"]->setValue(filterState.brightness);
    m_sliders["Saturation"]->setValue(filterState.saturation);
    m_sliders["Contrast"]->setValue(filterState.contrast);
    m_sliders["Blur"]->setValue(filterState.blur);
    m_sliders["Sharpen"]->setValue(filterState.sharpness);
    m_sliders["Temperature"]->setValue(filterState.temperature);
    m_sliders["Exposure"]->setValue(filterState.exposure);
    m_sliders["Gamma"]->setValue(filterState.gamma);
    m_sliders["Tint"]->setValue(filterState.tint);
    m_sliders["Vibrance"]->setValue(filterState.vibrance);
    m_sliders["Shadows"]->setValue(filterState.shadow);
    m_sliders["Highlights"]->setValue(filterState.highlight);
    m_sliders["Clarity"]->setValue(filterState.clarity);
    m_sliders["Vignette"]->setValue(filterState.vignette);
    m_sliders["Grain"]->setValue(filterState.grain);
    m_sliders["SplitToning"]->setValue(filterState.splitToning);
    m_sliders["Fade"]->setValue(filterState.fade);
    bwButton->setChecked(filterState.BWFilter);

    m_isUpdatingSlider = false;
    updateUndoRedoButtons();
}

void MainWindow::redo()
{
    m_isUpdatingSlider = true;
    undoRedoStack.redo();

    m_sliders["Brightness"]->setValue(filterState.brightness);
    m_sliders["Saturation"]->setValue(filterState.saturation);
    m_sliders["Contrast"]->setValue(filterState.contrast);
    m_sliders["Blur"]->setValue(filterState.blur);
    m_sliders["Sharpen"]->setValue(filterState.sharpness);
    m_sliders["Temperature"]->setValue(filterState.temperature);
    m_sliders["Exposure"]->setValue(filterState.exposure);
    m_sliders["Gamma"]->setValue(filterState.gamma);
    m_sliders["Tint"]->setValue(filterState.tint);
    m_sliders["Vibrance"]->setValue(filterState.vibrance);
    m_sliders["Shadows"]->setValue(filterState.shadow);
    m_sliders["Highlights"]->setValue(filterState.highlight);
    m_sliders["Clarity"]->setValue(filterState.clarity);
    m_sliders["Vignette"]->setValue(filterState.vignette);
    m_sliders["Grain"]->setValue(filterState.grain);
    m_sliders["SplitToning"]->setValue(filterState.splitToning);
    m_sliders["Fade"]->setValue(filterState.fade);
    bwButton->setChecked(filterState.BWFilter);

    m_isUpdatingSlider = false;
    updateUndoRedoButtons();
}

void MainWindow::changeFilterInt(int *target, int newValue)
{
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

void MainWindow::changeFilterBool(bool *target, bool newValue)
{
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

void MainWindow::rebuildPipeline()
{
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

void MainWindow::updateUndoRedoButtons()
{
    if (undoAction)
        undoAction->setEnabled(undoRedoStack.canUndo());
    if (redoAction)
        redoAction->setEnabled(undoRedoStack.canRedo());
}

void MainWindow::updateImage()
{
    if (graphicsView->getPixmap().isNull()) return;

    QImage result {pipeline.process(originalImage)};
    graphicsView->setPixmap(QPixmap::fromImage(result));
}

void MainWindow::setupShortcuts()
{
    // Additional keyboard shortcuts for quick navigation
    QShortcut* zoomIn {new QShortcut(QKeySequence("+"), this)};
    QShortcut* zoomIn2 {new QShortcut(QKeySequence("="), this)};

    connect(zoomIn, &QShortcut::activated, this, [this]() {
        graphicsView->scale(1.05, 1.05);
        updateZoomDisplay(graphicsView->transform().m11());
    });

    connect(zoomIn2, &QShortcut::activated, this, [this]() {
        graphicsView->scale(1.05, 1.05);
        updateZoomDisplay(graphicsView->transform().m11());
    });

    QShortcut* zoomOut {new QShortcut(QKeySequence("-"), this)};
    connect(zoomOut, &QShortcut::activated, this, [this]() {
        graphicsView->scale(0.95, 0.95);
        updateZoomDisplay(graphicsView->transform().m11());
    });

    QShortcut* cancelPanTool {new QShortcut(QKeySequence("Escape"), this)};
    connect(cancelPanTool, &QShortcut::activated, this, [this]() {
        m_isPanToolActive = false;
        panToolAction->setChecked(false);
        updatePanMode();
    });
}

void MainWindow::updatePanMode()
{
    graphicsView->setPanMode(m_isPanToolActive || m_isSpacePanActive);

    if (graphicsView->getPanMode()) {
        graphicsView->setCursor(Qt::OpenHandCursor);
    } else {
        graphicsView->unsetCursor();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        m_isSpacePanActive = true;
        updatePanMode();
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        m_isSpacePanActive = false;
        updatePanMode();
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::onCropFinished(const QRect &rect)
{
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

void MainWindow::updateZoomDisplay(double scale)
{
    int percent = static_cast<int>(scale * 100.0);
    if (zoomLabel) {
        zoomLabel->setText(QString::number(percent) + "%");
    }
}

QImage MainWindow::originalImage;
