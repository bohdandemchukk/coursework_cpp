#include "MainWindow.h"

#include <QFileDialog>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QShortcut>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QSpinBox>
#include <QTransform>
#include <QFileInfo>
#include <QPainter>
#include <algorithm>
#include <QToolButton>
#include <QMessageBox>

#include <memory>

#include "cropcommand.h"
#include "layercommands.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    applyGlobalStyle();
    createCentralCanvas();
    createActions();
    createTopBar();
    createFilterDock();
    createLayersDock();
    createFilterSections();
    setupShortcuts();

    updateUndoRedoButtons();

    m_layerManager.setOnChanged([this]() {
        if (m_brushTool) m_brushTool->setTargetImage(activeLayerImage());
        if (m_eraserTool) m_eraserTool->setTargetImage(activeLayerImage());
        if (m_layersPanel) m_layersPanel->setLayers(m_layerManager.layers(), m_layerManager.activeLayerIndex());
        updateComposite();
    });

    resize(1400, 900);
}

MainWindow::~MainWindow() = default;

void MainWindow::applyGlobalStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QMenuBar {
            background-color: #2b2b2b;
            color: #dddddd;
            border-bottom: 1px solid #1a1a1a;
        }
        QMenuBar::item {
            padding: 4px 12px;
            background: transparent;
        }
        QMenuBar::item:selected {
            background-color: #3a3a3a;
        }
        QMenu {
            background-color: #2b2b2b;
            color: #dddddd;
            border: 1px solid #1a1a1a;
        }
        QMenu::item:selected {
            background-color: #3a3a3a;
        }
        QToolBar {
            background-color: #2b2b2b;
            border: none;
            spacing: 5px;
            padding: 4px;
        }
        QToolButton {
            background-color: transparent;
            color: #dddddd;
            border: none;
            padding: 4px 6px;
        }
        QToolButton:hover {
            background-color: #3a3a3a;
            border-radius: 3px;
        }
        QDockWidget {
            background-color: #252525;
            color: #cccccc;
        }
        QDockWidget::title {
            background-color: #2b2b2b;
            padding: 6px;
            text-align: center;
        }
        QLabel {
            color: #cccccc;
        }

        QToolButton::menu-indicator {
            image: none;
            width: 0px;
        }

        QSpinBox::up-button,
        QSpinBox::down-button {
            width: 0px;
            height: 0px;
            border: none;
        }
        QSpinBox {
            padding-right: 0px;
        }
    )");
}

void MainWindow::createCentralCanvas()
{
    m_scene = new QGraphicsScene(this);
    m_graphicsView = new MyGraphicsView(m_scene, this);
    m_graphicsView->setStyleSheet(R"(
        QGraphicsView {
            background-color: #111111;
            border: none;
        }
    )");
    setCentralWidget(m_graphicsView);

    connect(m_graphicsView, &MyGraphicsView::cropFinished,
            this, &MainWindow::onCropFinished);

    connect(m_graphicsView, &MyGraphicsView::commandReady,
            this,
            [this](Command* raw)
            {
                if (!raw) return;

                std::unique_ptr<Command> cmd(raw);
                undoRedoStack.push(std::move(cmd));
                updateUndoRedoButtons();
            });


    connect(m_graphicsView, &MyGraphicsView::zoomChanged,
            this, [this](double scale) {
                if (!m_scaleSlider) return;
                int value{static_cast<int>(scale * 100.0)};
                value = std::clamp(value, m_scaleSlider->minimum(), m_scaleSlider->maximum());

                m_isUpdatingSlider = true;
                m_scaleSlider->setValue(value);
                m_isUpdatingSlider = false;
            });
}

void MainWindow::createActions()
{
    m_openAction = new QAction(tr("Open"), this);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openImage);

    m_saveAction = new QAction(tr("Save"), this);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::save);

    m_saveAsAction = new QAction(tr("Save As..."), this);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);

    m_exitAction = new QAction(tr("Exit"), this);
    connect(m_exitAction, &QAction::triggered, this, &QMainWindow::close);

    m_undoAction = new QAction(tr("Undo"), this);
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::doUndo);

    m_redoAction = new QAction(tr("Redo"), this);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::doRedo);

    m_cropAction = new QAction(tr("Crop"), this);
    connect(m_cropAction, &QAction::triggered, this, [this]() {
        if (m_graphicsView) {
            m_graphicsView->setCropMode(true);
        }
    });

    m_rotateLeftAction = new QAction(tr("Rotate 90° Left"), this);
    connect(m_rotateLeftAction, &QAction::triggered, this, &MainWindow::rotateLeft);

    m_rotateRightAction = new QAction(tr("Rotate 90° Right"), this);
    connect(m_rotateRightAction, &QAction::triggered, this, &MainWindow::rotateRight);

    m_flipHAction = new QAction(tr("Flip Horizontal"), this);
    connect(m_flipHAction, &QAction::triggered, this, &MainWindow::flipH);

    m_flipVAction = new QAction(tr("Flip Vertical"), this);
    connect(m_flipVAction, &QAction::triggered, this, &MainWindow::flipV);

    m_fitToScreenAction = new QAction(tr("Fit to Screen"), this);

    m_panAction = new QAction(tr("Pan"), this);
    m_panAction->setCheckable(true);

    connect(m_fitToScreenAction, &QAction::triggered, this, &MainWindow::fitToScreen);
    connect(m_panAction, &QAction::toggled, this, [this](bool checked) {
        m_isPanToolActive = checked;
        updatePanMode();
    });
}

void MainWindow::createTopBar()
{
    QToolBar* tb{new QToolBar(this)};
    tb->setIconSize(QSize(18, 18));
    tb->setMovable(false);
    tb->setFloatable(false);

    QToolButton* fileBtn{new QToolButton(tb)};
    fileBtn->setText("File");
    fileBtn->setPopupMode(QToolButton::InstantPopup);

    QMenu* fileMenu{new QMenu(this)};
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    fileBtn->setMenu(fileMenu);

    tb->addWidget(fileBtn);

    QToolButton* editBtn{new QToolButton(tb)};
    editBtn->setText("Edit");
    editBtn->setPopupMode(QToolButton::InstantPopup);

    QMenu* editMenu{new QMenu(this)};
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editBtn->setMenu(editMenu);

    tb->addWidget(editBtn);

    QToolButton* imgBtn{new QToolButton(tb)};
    imgBtn->setText("Image");
    imgBtn->setPopupMode(QToolButton::InstantPopup);

    QMenu* imgMenu{new QMenu(this)};
    imgMenu->addAction(m_cropAction);
    imgMenu->addSeparator();
    imgMenu->addAction(m_rotateLeftAction);
    imgMenu->addAction(m_rotateRightAction);
    imgMenu->addAction(m_flipHAction);
    imgMenu->addAction(m_flipVAction);
    imgBtn->setMenu(imgMenu);

    tb->addWidget(imgBtn);

    QToolButton* viewBtn{new QToolButton(tb)};
    viewBtn->setText("View");
    viewBtn->setPopupMode(QToolButton::InstantPopup);

    QMenu* viewMenu{new QMenu(this)};
    viewMenu->addAction(m_fitToScreenAction);
    viewMenu->addAction(m_panAction);
    viewBtn->setMenu(viewMenu);

    tb->addWidget(viewBtn);

    tb->addSeparator();

    tb->addAction(m_undoAction);
    tb->addAction(m_redoAction);

    tb->addSeparator();

    tb->addAction(m_cropAction);

    tb->addSeparator();

    QLabel* zoomLabel{new QLabel("Zoom:", tb)};
    tb->addWidget(zoomLabel);

    m_scaleSlider = new QSlider(Qt::Horizontal, tb);

    connect(m_scaleSlider, &QSlider::valueChanged, this, [this](int value) {
        if (m_isUpdatingSlider) return;
        if (!m_graphicsView) return;

        double scale{static_cast<double>(value) / 100.0};

        QTransform t{};
        t.scale(scale, scale);
        m_graphicsView->setTransform(t);
    });

    m_scaleSlider->setRange(10, 400);
    m_scaleSlider->setValue(100);
    m_scaleSlider->setFixedWidth(150);
    tb->addWidget(m_scaleSlider);

    tb->addAction(m_zoomInAction);

    addToolBar(Qt::TopToolBarArea, tb);
}

void MainWindow::createFilterDock()
{
    m_filterDock = new QDockWidget(tr("Filters"), this);
    m_filterDock->setFeatures(QDockWidget::DockWidgetMovable);
    m_filterDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QScrollArea* scrollArea{new QScrollArea(m_filterDock)};
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background-color: #252525;
            border: none;
        }
        QScrollBar:vertical {
            background-color: #1a1a1a;
            width: 12px;
        }
        QScrollBar::handle:vertical {
            background-color: #3a3a3a;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #4a4a4a;
        }
    )");

    m_filterPanel = new QWidget();
    QVBoxLayout* panelLayout{new QVBoxLayout(m_filterPanel)};
    panelLayout->setSpacing(0);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->addStretch();

    scrollArea->setWidget(m_filterPanel);
    m_filterDock->setWidget(scrollArea);

    addDockWidget(Qt::LeftDockWidgetArea, m_filterDock);
    m_filterDock->setMinimumWidth(320);
}

void MainWindow::createLayersDock()
{
    m_layersDock = new QDockWidget(tr("Layers"), this);
    m_layersPanel = new LayersPanel(this);
    m_layersDock->setWidget(m_layersPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_layersDock);

    connect(m_layersPanel, &LayersPanel::activeLayerChanged, this, &MainWindow::selectActiveLayer);
    connect(m_layersPanel, &LayersPanel::addLayerRequested, this, &MainWindow::handleAddLayer);
    connect(m_layersPanel, &LayersPanel::addAdjustmentLayerRequested, this, &MainWindow::handleAddAdjustmentLayer);
    connect(m_layersPanel, &LayersPanel::addImageLayerRequested, this, &MainWindow::handleAddImageLayer);
    connect(m_layersPanel, &LayersPanel::deleteLayerRequested, this, &MainWindow::handleDeleteLayer);
    connect(m_layersPanel, &LayersPanel::moveLayerRequested, this, &MainWindow::handleMoveLayer);
    connect(m_layersPanel, &LayersPanel::visibilityToggled, this, &MainWindow::handleVisibilityChanged);
    connect(m_layersPanel, &LayersPanel::opacityChanged, this, &MainWindow::handleOpacityChanged);
    connect(m_layersPanel,
            &LayersPanel::blendModeChanged,
            this,
            [this](int index, int mode)
            {
                auto cmd = std::make_unique<SetLayerBlendModeCommand>(
                    m_layerManager,
                    index,
                    static_cast<BlendMode>(mode)
                    );
                undoRedoStack.push(std::move(cmd));
                updateUndoRedoButtons();
            });


    if (m_layersPanel)
        m_layersPanel->setLayers(m_layerManager.layers(), m_layerManager.activeLayerIndex());
}


void MainWindow::createFilterSections()
{
    auto* layout{qobject_cast<QVBoxLayout*>(m_filterPanel->layout())};
    if (!layout) return;

    QLayoutItem* stretch{layout->takeAt(layout->count() - 1)};
    delete stretch;

    createToolsSection(layout);

    auto* basicSection{new CollapsibleSection("Basic", m_filterPanel)};
    auto* basicLayout{new QVBoxLayout()};
    basicLayout->setSpacing(5);

    m_exposureSlider = new FilterSlider("Exposure", -100, 100, 0);
    m_contrastSlider = new FilterSlider("Contrast", -100, 100, 0);
    m_brightnessSlider = new FilterSlider("Brightness", -100, 100, 0);
    m_highlightSlider = new FilterSlider("Highlights", -100, 100, 0);
    m_shadowSlider = new FilterSlider("Shadows", -100, 100, 0);
    m_claritySlider = new FilterSlider("Clarity", -100, 100, 0);

    basicLayout->addWidget(m_exposureSlider);
    basicLayout->addWidget(m_contrastSlider);
    basicLayout->addWidget(m_brightnessSlider);
    basicLayout->addWidget(m_highlightSlider);
    basicLayout->addWidget(m_shadowSlider);
    basicLayout->addWidget(m_claritySlider);

    basicSection->setContentLayout(basicLayout);
    layout->addWidget(basicSection);
    m_sections.append(basicSection);

    auto* colorSection{new CollapsibleSection("Color", m_filterPanel)};
    auto* colorLayout{new QVBoxLayout()};
    colorLayout->setSpacing(5);

    m_temperatureSlider = new FilterSlider("Temperature", -100, 100, 0);
    m_tintSlider        = new FilterSlider("Tint", -100, 100, 0);
    m_saturationSlider  = new FilterSlider("Saturation", -100, 100, 0);
    m_vibranceSlider    = new FilterSlider("Vibrance", -100, 100, 0);

    colorLayout->addWidget(m_temperatureSlider);
    colorLayout->addWidget(m_tintSlider);
    colorLayout->addWidget(m_saturationSlider);
    colorLayout->addWidget(m_vibranceSlider);

    colorSection->setContentLayout(colorLayout);
    layout->addWidget(colorSection);
    m_sections.append(colorSection);

    auto* effectsSection{new CollapsibleSection("Effects", m_filterPanel)};
    auto* effectsLayout{new QVBoxLayout()};
    effectsLayout->setSpacing(5);

    m_splitToningSlider = new FilterSlider("Split Toning", 0, 100, 0);
    m_vignetteSlider    = new FilterSlider("Vignette", 0, 100, 0);
    m_grainSlider       = new FilterSlider("Grain", 0, 100, 0);
    m_fadeSlider        = new FilterSlider("Fade", 0, 100, 0);

    effectsLayout->addWidget(m_splitToningSlider);
    effectsLayout->addWidget(m_vignetteSlider);
    effectsLayout->addWidget(m_grainSlider);
    effectsLayout->addWidget(m_fadeSlider);

    effectsSection->setContentLayout(effectsLayout);
    layout->addWidget(effectsSection);
    m_sections.append(effectsSection);

    auto* detailSection{new CollapsibleSection("Detail", m_filterPanel)};
    auto* detailLayout{new QVBoxLayout()};
    detailLayout->setSpacing(5);

    m_sharpenSlider = new FilterSlider("Sharpen", 0, 100, 0);
    m_blurSlider    = new FilterSlider("Blur", 0, 100, 0);

    detailLayout->addWidget(m_sharpenSlider);
    detailLayout->addWidget(m_blurSlider);

    detailSection->setContentLayout(detailLayout);
    layout->addWidget(detailSection);
    m_sections.append(detailSection);

    auto* transformSection{new CollapsibleSection("Transform", m_filterPanel)};
    auto* transformLayout{new QVBoxLayout()};
    transformLayout->setSpacing(5);
    transformLayout->setContentsMargins(10, 5, 10, 5);

    QPushButton* rotateRBtn{new QPushButton("Rotate 90° Right")};
    rotateRBtn->setStyleSheet("QPushButton { padding: 8px; background-color: #3a3a3a; border-radius: 3px; } QPushButton:hover { background-color: #4a4a4a; }");
    transformLayout->addWidget(rotateRBtn);

    QPushButton* rotateLBtn{new QPushButton("Rotate 90° Left")};
    rotateLBtn->setStyleSheet("QPushButton { padding: 8px; background-color: #3a3a3a; border-radius: 3px; } QPushButton:hover { background-color: #4a4a4a; }");
    transformLayout->addWidget(rotateLBtn);

    QPushButton* flipHBtn{new QPushButton("Flip Horizontal")};
    flipHBtn->setStyleSheet("QPushButton { padding: 8px; background-color: #3a3a3a; border-radius: 3px; } QPushButton:hover { background-color: #4a4a4a; }");
    transformLayout->addWidget(flipHBtn);

    QPushButton* flipVBtn{new QPushButton("Flip Vertical")};
    flipVBtn->setStyleSheet("QPushButton { padding: 8px; background-color: #3a3a3a; border-radius: 3px; } QPushButton:hover { background-color: #4a4a4a; }");
    transformLayout->addWidget(flipVBtn);

    m_bwCheckbox = new QCheckBox("Black & White");
    m_bwCheckbox->setStyleSheet("QCheckBox { color: #aaaaaa; padding: 5px; }");
    transformLayout->addWidget(m_bwCheckbox);

    m_gammaSlider = new FilterSlider("Gamma", -100, 100, 0);
    transformLayout->addWidget(m_gammaSlider);

    transformSection->setContentLayout(transformLayout);
    layout->addWidget(transformSection);
    m_sections.append(transformSection);

    layout->addStretch();

    for (CollapsibleSection* section : m_sections) {
        connect(section, &CollapsibleSection::toggled, this, [this, section](bool expanded) {
            if (!expanded) return;
            for (CollapsibleSection* other : m_sections) {
                if (other != section && other->isExpanded()) {
                    other->toggle();
                }
            }
        });
    }

    auto connectLayerSlider =
        [this](FilterSlider* slider,
               std::function<void(FilterPipeline&, int)> apply)
    {
        if (!slider) return;

        connect(slider, &FilterSlider::sliderReleased,
                this, [this, apply](int value)
                {
                    if (m_isUpdatingSlider) return;

                    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(m_layerManager.activeLayer());
                    if (!layer) return;

                    int index = m_layerManager.activeLayerIndex();

                    FilterPipeline before = layer->pipeline();
                    FilterPipeline after  = before;

                    apply(after, value);

                    auto cmd = std::make_unique<ChangeLayerPipelineCommand>(
                        m_layerManager,
                        index,
                        std::move(before),
                        std::move(after)
                        );

                    undoRedoStack.push(std::move(cmd));
                    updateUndoRedoButtons();
                });
    };

    connectLayerSlider(m_exposureSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<ExposureFilter>(v);
                       });

    connectLayerSlider(m_contrastSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<ContrastFilter>(v);
                       });

    connectLayerSlider(m_brightnessSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<HighlightFilter>(v);
                       });

    connectLayerSlider(m_shadowSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<ShadowFilter>(v);
                       });

    connectLayerSlider(m_highlightSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<ExposureFilter>(v);
                       });

    connectLayerSlider(m_claritySlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<ClarityFilter>(v);
                       });

    connectLayerSlider(m_temperatureSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<TemperatureFilter>(v);
                       });

    connectLayerSlider(m_tintSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<TintFilter>(v);
                       });

    connectLayerSlider(m_saturationSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<SaturationFilter>(v);
                       });

    connectLayerSlider(m_vibranceSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<VibranceFilter>(v);
                       });

    connectLayerSlider(m_splitToningSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<SplitToningFilter>(v);
                       });

    connectLayerSlider(m_vignetteSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<VignetteFilter>(v);
                       });

    connectLayerSlider(m_fadeSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<FadeFilter>(v);
                       });

    connectLayerSlider(m_grainSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<GrainFilter>(v);
                       });

    connectLayerSlider(m_sharpenSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<SharpenFilter>(v);
                       });

    connectLayerSlider(m_blurSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<BlurFilter>(v);
                       });

    connectLayerSlider(m_gammaSlider,
                       [](FilterPipeline& p, int v) {
                           p.setOrReplace<GammaFilter>(v);
                       });




    connect(m_bwCheckbox, &QCheckBox::toggled, this,
            [this](bool checked)
            {
                if (m_isUpdatingSlider) return;

                auto layer = m_layerManager.activeLayer();
                auto adj = std::dynamic_pointer_cast<AdjustmentLayer>(layer);
                if (!adj) return;

                int index = m_layerManager.activeLayerIndex();

                FilterPipeline before = adj->pipeline();
                FilterPipeline after  = before;

                if (checked)
                    after.setOrReplace<BWFilter>(true);
                else
                    after.remove<BWFilter>();

                auto cmd = std::make_unique<ChangeLayerPipelineCommand>(
                    m_layerManager,
                    index,
                    std::move(before),
                    std::move(after)
                    );

                undoRedoStack.push(std::move(cmd));
                updateUndoRedoButtons();
            }
            );



    connect(rotateRBtn, &QPushButton::clicked, this, &MainWindow::rotateRight);
    connect(rotateLBtn, &QPushButton::clicked, this, &MainWindow::rotateLeft);
    connect(flipHBtn,   &QPushButton::clicked, this, &MainWindow::flipH);
    connect(flipVBtn,   &QPushButton::clicked, this, &MainWindow::flipV);
}

void MainWindow::createToolsSection(QVBoxLayout* layout)
{
    m_toolsSection = new CollapsibleSection("Tools", m_filterPanel);
    auto* toolsLayout = new QVBoxLayout();
    toolsLayout->setSpacing(8);

    auto* brushRow = new QHBoxLayout();
    auto* brushButton = new QPushButton("Brush");
    brushButton->setCheckable(true);
    auto* eraserButton = new QPushButton("Eraser");
    eraserButton->setCheckable(true);

    auto setExclusive = [brushButton, eraserButton](QPushButton* pressed) {
        brushButton->setChecked(pressed == brushButton);
        eraserButton->setChecked(pressed == eraserButton);
    };

    connect(brushButton, &QPushButton::clicked, this, [this, setExclusive, brushButton](bool checked) {
        if (!checked) {
            brushButton->setChecked(true);
        }
        setExclusive(brushButton);
        setActiveTool(m_brushTool.get());
    });

    connect(eraserButton, &QPushButton::clicked, this, [this, setExclusive, eraserButton](bool checked) {
        if (!checked) {
            eraserButton->setChecked(true);
        }
        setExclusive(eraserButton);
        setActiveTool(m_eraserTool.get());
    });

    brushRow->addWidget(brushButton);
    brushRow->addWidget(eraserButton);

    auto* sizeLayout = new QHBoxLayout();
    auto* sizeLabel = new QLabel("Size:");
    m_brushSizeSpin = new QSpinBox();
    m_brushSizeSpin->setRange(1, 200);
    m_brushSizeSpin->setValue(10);
    connect(m_brushSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (m_brushTool) m_brushTool->setBrushSize(value);
        if (m_eraserTool) m_eraserTool->setBrushSize(value);
    });

    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(m_brushSizeSpin);

    auto* colorLayout = new QHBoxLayout();
    auto* colorLabel = new QLabel("Color:");
    m_colorButton = new QPushButton();
    m_colorButton->setFixedSize(32, 24);
    updateBrushColorButton();
    connect(m_colorButton, &QPushButton::clicked, this, [this]() {
        QColor chosen = QColorDialog::getColor(m_brushColor, this, "Select Brush Color");
        if (!chosen.isValid()) return;
        m_brushColor = chosen;
        if (m_brushTool) m_brushTool->setColor(m_brushColor);
        updateBrushColorButton();
    });
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(m_colorButton);

    toolsLayout->addLayout(brushRow);
    toolsLayout->addLayout(sizeLayout);
    toolsLayout->addLayout(colorLayout);

    m_toolsSection->setContentLayout(toolsLayout);
    layout->addWidget(m_toolsSection);
    m_sections.append(m_toolsSection);

    brushButton->setChecked(true);
}

void MainWindow::setupShortcuts()
{
    auto* zoomInShortcut{new QShortcut(QKeySequence("+"), this)};
    auto* zoomInShortcut2{new QShortcut(QKeySequence("="), this)};
    connect(zoomInShortcut, &QShortcut::activated, this, [this]() {
        if (!m_scaleSlider) return;
        m_scaleSlider->setValue(m_scaleSlider->value() + 5);
    });
    connect(zoomInShortcut2, &QShortcut::activated, this, [this]() {
        if (!m_scaleSlider) return;
        m_scaleSlider->setValue(m_scaleSlider->value() + 5);
    });

    auto* zoomOutShortcut{new QShortcut(QKeySequence("-"), this)};
    connect(zoomOutShortcut, &QShortcut::activated, this, [this]() {
        if (!m_scaleSlider) return;
        m_scaleSlider->setValue(m_scaleSlider->value() - 5);
    });

    auto* panToolShortcut{new QShortcut(QKeySequence("H"), this)};
    connect(panToolShortcut, &QShortcut::activated, this, [this]() {
        m_isPanToolActive = !m_isPanToolActive;
        updatePanMode();
    });

    auto* cancelPanShortcut{new QShortcut(QKeySequence("Escape"), this)};
    connect(cancelPanShortcut, &QShortcut::activated, this, [this]() {
        m_isPanToolActive = false;
        updatePanMode();
    });

    m_openAction->setShortcut(QKeySequence("Ctrl+O"));
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    m_undoAction->setShortcut(QKeySequence("Ctrl+Z"));
    m_redoAction->setShortcuts({ QKeySequence("Ctrl+Y"), QKeySequence("Ctrl+Shift+Z") });
    m_cropAction->setShortcut(QKeySequence("C"));
    m_rotateLeftAction->setShortcut(QKeySequence("Ctrl+["));
    m_rotateRightAction->setShortcut(QKeySequence("Ctrl+]"));
    m_fitToScreenAction->setShortcut(QKeySequence("Ctrl+0"));
    m_flipHAction->setShortcut(QKeySequence("Ctrl+H"));
    m_flipVAction->setShortcut(QKeySequence("Ctrl+V"));
    m_fitToScreenAction->setShortcut(QKeySequence("Ctrl+0"));
    m_saveAction->setShortcut(QKeySequence("Ctrl+S"));
    m_saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
}

void MainWindow::initializeTools()
{
    m_brushTool = std::make_unique<BrushTool>(activeLayerImage(), [this]() { updateComposite(); }, m_graphicsView);
    m_eraserTool = std::make_unique<EraserTool>(activeLayerImage(), [this]() { updateComposite(); }, m_graphicsView);

    int size = m_brushSizeSpin ? m_brushSizeSpin->value() : 10;
    m_brushTool->setBrushSize(size);
    m_eraserTool->setBrushSize(size);

    m_brushTool->setColor(m_brushColor);

    setActiveTool(m_brushTool.get());
}

void MainWindow::setActiveTool(Tool* tool)
{
    m_activeTool = tool;
    if (m_graphicsView) {
        m_graphicsView->setActiveTool(m_activeTool);
    }
}

void MainWindow::updateBrushColorButton()
{
    if (!m_colorButton) return;
    QString style = QString("background-color: %1; border: 1px solid #444; border-radius: 3px;")
                        .arg(m_brushColor.name());
    m_colorButton->setStyleSheet(style);
}

void MainWindow::updatePanMode()
{
    if (!m_graphicsView) return;

    bool pan{m_isPanToolActive || m_isSpacePanActive};
    m_graphicsView->setPanMode(pan);

    if (pan) {
        m_graphicsView->setCursor(Qt::OpenHandCursor);
    } else {
        m_graphicsView->unsetCursor();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_isSpacePanActive = true;
        updatePanMode();
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_isSpacePanActive = false;
        updatePanMode();
    }
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::openImage()
{
    const QString fileName{QFileDialog::getOpenFileName(
        this,
        tr("Open Image"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp)")
        )};

    if (fileName.isEmpty()) return;

    m_currentFilePath = fileName;

    QImage img(fileName);
    if (img.isNull()) return;

    m_layerManager = LayerManager{};
    m_layerManager.setCanvasSize(img.size());
    m_layerManager.setOnChanged([this]() {
        if (m_brushTool) m_brushTool->setTargetImage(activeLayerImage());
        if (m_eraserTool) m_eraserTool->setTargetImage(activeLayerImage());
        if (m_layersPanel) m_layersPanel->setLayers(m_layerManager.layers(), m_layerManager.activeLayerIndex());
        updateComposite();
    });

    auto baseLayer = std::make_shared<PixelLayer>(tr("Background"), img.convertToFormat(QImage::Format_ARGB32));
    m_layerManager.addLayer(baseLayer);
    m_layerManager.setActiveLayerIndex(0);
    if (m_graphicsView) {
        m_graphicsView->setPixmap(QPixmap::fromImage(compositeWithFilters()));
    }

    filterState = FilterState{};
    pipeline.clear();
    undoRedoStack.clear();

    m_isUpdatingSlider = true;
    if (m_exposureSlider)   m_exposureSlider->setValue(0);
    if (m_contrastSlider)   m_contrastSlider->setValue(0);
    if (m_brightnessSlider) m_brightnessSlider->setValue(0);
    if (m_highlightSlider)  m_highlightSlider->setValue(0);
    if (m_shadowSlider)     m_shadowSlider->setValue(0);
    if (m_claritySlider)    m_claritySlider->setValue(0);

    if (m_temperatureSlider) m_temperatureSlider->setValue(0);
    if (m_tintSlider)        m_tintSlider->setValue(0);
    if (m_saturationSlider)  m_saturationSlider->setValue(0);
    if (m_vibranceSlider)    m_vibranceSlider->setValue(0);

    if (m_splitToningSlider) m_splitToningSlider->setValue(0);
    if (m_vignetteSlider)    m_vignetteSlider->setValue(0);
    if (m_grainSlider)       m_grainSlider->setValue(0);
    if (m_fadeSlider)        m_fadeSlider->setValue(0);

    if (m_sharpenSlider) m_sharpenSlider->setValue(0);
    if (m_blurSlider)    m_blurSlider->setValue(0);
    if (m_gammaSlider)   m_gammaSlider->setValue(0);

    if (m_bwCheckbox) m_bwCheckbox->setChecked(false);

    if (m_scaleSlider) m_scaleSlider->setValue(100);
    m_isUpdatingSlider = false;

    fitToScreen();

    initializeTools();
    rebuildPipeline();
    updateUndoRedoButtons();
}

void MainWindow::onCropFinished(const QRect& rect)
{
    QImage* target = activeLayerImage();
    if (!target || target->isNull()) return;

    auto cmd{std::make_unique<CropCommand>(
        target,
        rect,
        [this]() {
            this->updateComposite();
        }
        )};

    undoRedoStack.push(std::move(cmd));
    updateUndoRedoButtons();
}

void MainWindow::fitToScreen()
{
    if (!m_graphicsView || !m_scaleSlider) return;

    const QSize imgSize{m_layerManager.canvasSize()};
    const QSize viewSize{m_graphicsView->viewport()->size()};

    if (imgSize.isEmpty() || viewSize.isEmpty()) return;

    const double scaleX{static_cast<double>(viewSize.width())  / imgSize.width()};
    const double scaleY{static_cast<double>(viewSize.height()) / imgSize.height()};
    const double scale{std::min(scaleX, scaleY)};

    int sliderValue{static_cast<int>(scale * 100.0)};
    sliderValue = std::clamp(sliderValue, m_scaleSlider->minimum(), m_scaleSlider->maximum());

    m_isUpdatingSlider = true;
    m_scaleSlider->setValue(sliderValue);
    m_isUpdatingSlider = false;

    QTransform t{};
    t.scale(scale, scale);
    m_graphicsView->setTransform(t);
}

void MainWindow::rotateLeft()
{
    changeFilterInt(&filterState.rotateAngle, filterState.rotateAngle - 90);
}

void MainWindow::rotateRight()
{
    changeFilterInt(&filterState.rotateAngle, filterState.rotateAngle + 90);
}

void MainWindow::flipH()
{
    changeFilterBool(&filterState.flipH, !filterState.flipH);
}

void MainWindow::flipV()
{
    changeFilterBool(&filterState.flipV, !filterState.flipV);
}

void MainWindow::changeFilterInt(int* target, int newValue)
{
    if (!target) return;
    int oldValue{*target};

    auto command{std::make_unique<ChangeFilterIntCommand>(
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

void MainWindow::changeFilterBool(bool* target, bool newValue)
{
    if (!target) return;
    bool oldValue{*target};

    auto command{std::make_unique<ChangeFilterBoolCommand>(
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
    pipeline.addFilter(std::make_unique<FlipFilter>(FlipFilter::Direction::Vertical,   filterState.flipV));

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

    updateComposite();
}

void MainWindow::changeLayerFilters(std::function<void(FilterPipeline&)> edit)
{
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(m_layerManager.activeLayer());
    if (!layer) return;

    int index = m_layerManager.activeLayerIndex();

    FilterPipeline before = layer->pipeline();
    FilterPipeline after = before;

    edit(after);

    auto cmd = std::make_unique<ChangeLayerPipelineCommand>(
        m_layerManager,
        index,
        std::move(before),
        std::move(after)
        );

    undoRedoStack.push(std::move(cmd));
}

void MainWindow::syncFilterUIFromActiveLayer()
{
     auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(m_layerManager.activeLayer());
    m_isUpdatingSlider = true;

     if (!layer)
     {
         setFilterControlsEnabled(false);
         if (m_bwCheckbox) m_bwCheckbox->setChecked(false);
         if (m_exposureSlider)   m_exposureSlider->setValue(0);
         if (m_contrastSlider)   m_contrastSlider->setValue(0);
         if (m_brightnessSlider) m_brightnessSlider->setValue(0);
         if (m_highlightSlider)  m_highlightSlider->setValue(0);
         if (m_shadowSlider)     m_shadowSlider->setValue(0);
         if (m_claritySlider)    m_claritySlider->setValue(0);
         if (m_temperatureSlider) m_temperatureSlider->setValue(0);
         if (m_tintSlider)        m_tintSlider->setValue(0);
         if (m_saturationSlider)  m_saturationSlider->setValue(0);
         if (m_vibranceSlider)    m_vibranceSlider->setValue(0);
         if (m_splitToningSlider) m_splitToningSlider->setValue(0);
         if (m_vignetteSlider)    m_vignetteSlider->setValue(0);
         if (m_grainSlider)       m_grainSlider->setValue(0);
         if (m_fadeSlider)        m_fadeSlider->setValue(0);
         if (m_sharpenSlider)     m_sharpenSlider->setValue(0);
         if (m_blurSlider)        m_blurSlider->setValue(0);
         if (m_gammaSlider)       m_gammaSlider->setValue(0);
         m_isUpdatingSlider = false;
         return;
     }

     setFilterControlsEnabled(true);
     auto& p = layer->pipeline();

     m_exposureSlider->setValue(p.find<ExposureFilter>() ? p.find<ExposureFilter>()->getExposure() : 0);
     m_temperatureSlider->setValue(p.find<TemperatureFilter>() ? p.find<TemperatureFilter>()->getTemperature() : 0);
     m_blurSlider->setValue(p.find<BlurFilter>() ? p.find<BlurFilter>()->getBlur() : 0);
     m_brightnessSlider->setValue(p.find<BrightnessFilter>() ? p.find<BrightnessFilter>()->getBrightness() : 0);
     m_claritySlider->setValue(p.find<ClarityFilter>() ? p.find<ClarityFilter>()->getClarity() : 0);
     m_contrastSlider->setValue(p.find<ContrastFilter>() ? p.find<ContrastFilter>()->getContrast() : 0);
     m_fadeSlider->setValue(p.find<FadeFilter>() ? p.find<FadeFilter>()->getFade() : 0);
     m_gammaSlider->setValue(p.find<GammaFilter>() ? p.find<GammaFilter>()->getGamma() : 0);
     m_grainSlider->setValue(p.find<GrainFilter>() ? p.find<GrainFilter>()->getGrain() : 0);
     m_highlightSlider->setValue(p.find<HighlightFilter>() ? p.find<HighlightFilter>()->getHighlight() : 0);
     m_saturationSlider->setValue(p.find<SaturationFilter>() ? p.find<SaturationFilter>()->getSaturation() : 0);
     m_shadowSlider->setValue(p.find<ShadowFilter>() ? p.find<ShadowFilter>()->getShadow() : 0);
     m_sharpenSlider->setValue(p.find<SharpenFilter>() ? p.find<SharpenFilter>()->getSharpness() : 0);
     m_splitToningSlider->setValue(p.find<SplitToningFilter>() ? p.find<SplitToningFilter>()->getSplitToning() : 0);
     m_tintSlider->setValue(p.find<TintFilter>() ? p.find<TintFilter>()->getTint() : 0);
     m_vibranceSlider->setValue(p.find<VibranceFilter>() ? p.find<VibranceFilter>()->getVibrance() : 0);
     m_vignetteSlider->setValue(p.find<VignetteFilter>() ? p.find<VignetteFilter>()->getVignette() : 0);

     if (m_bwCheckbox)
         m_bwCheckbox->setChecked(p.find<BWFilter>() != nullptr);

     m_isUpdatingSlider = false;

}



QImage* MainWindow::activeLayerImage()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_layerManager.activeLayer());
    if (!layer)
        return nullptr;
    return &layer->image();
}

void MainWindow::selectActiveLayer(int index)
{
    m_layerManager.setActiveLayerIndex(index);

    syncFilterUIFromActiveLayer();

    QImage* target = activeLayerImage();
    if (m_brushTool) m_brushTool->setTargetImage(target);
    if (m_eraserTool) m_eraserTool->setTargetImage(target);
}

bool MainWindow::isActiveAdjustmentLayer() const
{
    auto layer = m_layerManager.activeLayer();
    auto adj = std::dynamic_pointer_cast<const AdjustmentLayer>(layer);
    return adj != nullptr;
}

void MainWindow::setFilterControlsEnabled(bool enabled)
{
    const auto setEnabledIf = [enabled](QWidget* w) { if (w) w->setEnabled(enabled); };
    setEnabledIf(m_exposureSlider);
    setEnabledIf(m_contrastSlider);
    setEnabledIf(m_brightnessSlider);
    setEnabledIf(m_highlightSlider);
    setEnabledIf(m_shadowSlider);
    setEnabledIf(m_claritySlider);
    setEnabledIf(m_temperatureSlider);
    setEnabledIf(m_tintSlider);
    setEnabledIf(m_saturationSlider);
    setEnabledIf(m_vibranceSlider);
    setEnabledIf(m_splitToningSlider);
    setEnabledIf(m_vignetteSlider);
    setEnabledIf(m_grainSlider);
    setEnabledIf(m_fadeSlider);
    setEnabledIf(m_sharpenSlider);
    setEnabledIf(m_blurSlider);
    setEnabledIf(m_gammaSlider);
    setEnabledIf(m_bwCheckbox);
}

QImage MainWindow::compositeWithFilters()
{
    QImage base = m_layerManager.composite();
    if (base.isNull())
        return base;

    return pipeline.process(base);
}



void MainWindow::updateActiveLayerImage(const QImage &image)
{
    auto active = std::dynamic_pointer_cast<PixelLayer>(m_layerManager.activeLayer());
    if (!active)
        return;

    active->setImage(image);
    updateComposite();
}

void MainWindow::updateComposite()
{
    if (!m_graphicsView)
        return;

    QImage result = compositeWithFilters();
    if (!result.isNull())
    {
        m_graphicsView->setPixmap(QPixmap::fromImage(result));
    }
}

void MainWindow::handleAddLayer()
{
    if (!m_layerManager.canvasSize().isValid())
        return;

    QImage newImage(m_layerManager.canvasSize(), QImage::Format_ARGB32_Premultiplied);
    newImage.fill(Qt::transparent);
    QString name = tr("Layer %1").arg(m_layerManager.layerCount() + 1);

    auto layer = std::make_shared<PixelLayer>(name, newImage);
    auto command = std::make_unique<AddLayerCommand>(m_layerManager, layer);
    undoRedoStack.push(std::move(command));
    m_layerManager.setActiveLayerIndex(m_layerManager.layerCount() - 1);
    updateUndoRedoButtons();
}

void MainWindow::handleAddAdjustmentLayer()
{
    if (!m_layerManager.canvasSize().isValid())
        return;

    QString name = tr("Adjustment %1")
                       .arg(m_layerManager.layerCount() + 1);

    auto adjLayer = std::make_shared<AdjustmentLayer>(name);

    int insertIndex = m_layerManager.activeLayerIndex() + 1;

    auto cmd = std::make_unique<AddLayerCommand>(
        m_layerManager,
        adjLayer,
        insertIndex
        );

    undoRedoStack.push(std::move(cmd));
    m_layerManager.setActiveLayerIndex(insertIndex);

    syncFilterUIFromActiveLayer();
    updateUndoRedoButtons();
}

QImage MainWindow::prepareImageForCanvas(const QImage& source, const QSize& canvasSize) const
{
    QImage converted = source.convertToFormat(QImage::Format_ARGB32);
    if (!canvasSize.isValid())
        return converted;

    if (converted.size() == canvasSize)
        return converted;

    QImage result(canvasSize, QImage::Format_ARGB32);
    result.fill(Qt::transparent);

    QImage scaled = converted.scaled(canvasSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const QPoint topLeft((canvasSize.width() - scaled.width()) / 2,
                         (canvasSize.height() - scaled.height()) / 2);

    QPainter painter(&result);
    painter.drawImage(topLeft, scaled);

    return result;
}

void MainWindow::handleAddImageLayer()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Add Image Layer"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.bmp)")
        );

    if (filePath.isEmpty())
        return;

    if (!m_layerManager.canvasSize().isValid())
        return;

    QImage loaded(filePath);
    if (loaded.isNull())
        return;

    const QImage prepared = prepareImageForCanvas(loaded, m_layerManager.canvasSize());

    const QString baseName = QFileInfo(filePath).completeBaseName();
    const QString layerName = baseName.isEmpty()
                                  ? tr("Image Layer %1").arg(m_layerManager.layerCount() + 1)
                                  : baseName;

    const int insertIndex = m_layerManager.activeLayerIndex() + 1;
    auto layer = std::make_shared<PixelLayer>(layerName, prepared);
    auto command = std::make_unique<AddLayerCommand>(m_layerManager, layer, insertIndex);
    undoRedoStack.push(std::move(command));

    m_layerManager.setActiveLayerIndex(insertIndex);
    updateUndoRedoButtons();
}


void MainWindow::handleDeleteLayer(int managerIndex)
{
    if (managerIndex < 0 || managerIndex >= m_layerManager.layerCount())
        return;

    auto command = std::make_unique<RemoveLayerCommand>(m_layerManager, managerIndex);
    undoRedoStack.push(std::move(command));
    updateUndoRedoButtons();
}

void MainWindow::handleMoveLayer(int from, int to)
{
    int count = m_layerManager.layerCount();
    if (from < 0 || from >= count)
        return;

    to = std::clamp(to, 0, count - 1);
    if (from == to)
        return;

    auto command = std::make_unique<MoveLayerCommand>(m_layerManager, from, to);
    undoRedoStack.push(std::move(command));
    updateUndoRedoButtons();
}

void MainWindow::handleVisibilityChanged(int managerIndex, bool visible)
{
    if (managerIndex < 0 || managerIndex >= m_layerManager.layerCount())
        return;

    auto command = std::make_unique<SetLayerVisibilityCommand>(m_layerManager, managerIndex, visible);
    undoRedoStack.push(std::move(command));
    updateUndoRedoButtons();
}

void MainWindow::handleOpacityChanged(int managerIndex, float opacity)
{
    if (managerIndex < 0 || managerIndex >= m_layerManager.layerCount())
        return;

    auto command = std::make_unique<SetLayerOpacityCommand>(m_layerManager, managerIndex, opacity);
    undoRedoStack.push(std::move(command));
    updateUndoRedoButtons();
}

void MainWindow::doUndo()
{
    if (!undoRedoStack.canUndo()) return;

    m_isUpdatingSlider = true;
    undoRedoStack.undo();

    if (m_exposureSlider)   m_exposureSlider->setValue(filterState.exposure);
    if (m_contrastSlider)   m_contrastSlider->setValue(filterState.contrast);
    if (m_brightnessSlider) m_brightnessSlider->setValue(filterState.brightness);
    if (m_highlightSlider)  m_highlightSlider->setValue(filterState.highlight);
    if (m_shadowSlider)     m_shadowSlider->setValue(filterState.shadow);
    if (m_claritySlider)    m_claritySlider->setValue(filterState.clarity);

    if (m_temperatureSlider) m_temperatureSlider->setValue(filterState.temperature);
    if (m_tintSlider)        m_tintSlider->setValue(filterState.tint);
    if (m_saturationSlider)  m_saturationSlider->setValue(filterState.saturation);
    if (m_vibranceSlider)    m_vibranceSlider->setValue(filterState.vibrance);

    if (m_splitToningSlider) m_splitToningSlider->setValue(filterState.splitToning);
    if (m_vignetteSlider)    m_vignetteSlider->setValue(filterState.vignette);
    if (m_grainSlider)       m_grainSlider->setValue(filterState.grain);
    if (m_fadeSlider)        m_fadeSlider->setValue(filterState.fade);

    if (m_sharpenSlider) m_sharpenSlider->setValue(filterState.sharpness);
    if (m_blurSlider)    m_blurSlider->setValue(filterState.blur);
    if (m_gammaSlider)   m_gammaSlider->setValue(filterState.gamma);

    if (m_bwCheckbox) m_bwCheckbox->setChecked(filterState.BWFilter);

    m_isUpdatingSlider = false;
    updateUndoRedoButtons();
}

void MainWindow::doRedo()
{
    if (!undoRedoStack.canRedo()) return;

    m_isUpdatingSlider = true;
    undoRedoStack.redo();

    if (m_exposureSlider)   m_exposureSlider->setValue(filterState.exposure);
    if (m_contrastSlider)   m_contrastSlider->setValue(filterState.contrast);
    if (m_brightnessSlider) m_brightnessSlider->setValue(filterState.brightness);
    if (m_highlightSlider)  m_highlightSlider->setValue(filterState.highlight);
    if (m_shadowSlider)     m_shadowSlider->setValue(filterState.shadow);
    if (m_claritySlider)    m_claritySlider->setValue(filterState.clarity);

    if (m_temperatureSlider) m_temperatureSlider->setValue(filterState.temperature);
    if (m_tintSlider)        m_tintSlider->setValue(filterState.tint);
    if (m_saturationSlider)  m_saturationSlider->setValue(filterState.saturation);
    if (m_vibranceSlider)    m_vibranceSlider->setValue(filterState.vibrance);

    if (m_splitToningSlider) m_splitToningSlider->setValue(filterState.splitToning);
    if (m_vignetteSlider)    m_vignetteSlider->setValue(filterState.vignette);
    if (m_grainSlider)       m_grainSlider->setValue(filterState.grain);
    if (m_fadeSlider)        m_fadeSlider->setValue(filterState.fade);

    if (m_sharpenSlider) m_sharpenSlider->setValue(filterState.sharpness);
    if (m_blurSlider)    m_blurSlider->setValue(filterState.blur);
    if (m_gammaSlider)   m_gammaSlider->setValue(filterState.gamma);

    if (m_bwCheckbox) m_bwCheckbox->setChecked(filterState.BWFilter);

    m_isUpdatingSlider = false;
    updateUndoRedoButtons();
}

void MainWindow::updateUndoRedoButtons()
{
    if (m_undoAction) m_undoAction->setEnabled(undoRedoStack.canUndo());
    if (m_redoAction) m_redoAction->setEnabled(undoRedoStack.canRedo());
}

void MainWindow::save() {

    if (m_currentFilePath.isEmpty()) {
        saveAs();
        return;
    }

    QPixmap pixmap {m_graphicsView->getPixmap()};

    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Error", "No image to save");
    }

    if (!pixmap.save(m_currentFilePath)) {
        QMessageBox::warning(this, "Error", "Failed to save image");
    }
}

void MainWindow::saveAs() {

    if (m_currentFilePath.isEmpty()) return;

    QString fileName {QFileDialog::getSaveFileName(
        this,
        "Save Image As",
        "",
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"
        )};

    if (fileName.isEmpty()) return;


    QPixmap pixmap {m_graphicsView->getPixmap()};

    if (!pixmap.save(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to save file");
        return;
    }
}


