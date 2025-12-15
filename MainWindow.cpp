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
#include <QActionGroup>

#include <memory>
\
#include "changelayerpipelinecommand.h"
#include "rotatelayercommand.h"
#include "fliplayercommand.h"
#include "imageio.h"
#include "cropcommand.h"
#include "layercommands.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    applyGlobalStyle();
    createCentralCanvas();
    createActions();
    createTopBar();
    createFilterDock();
    createLayersDock();
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
        /* =========================
           GLOBAL
        ==========================*/
        QWidget {
            background-color: #2a2f33;
            color: #d6d6d6;
            font-family: 'Segoe UI', 'Inter', sans-serif;
            font-size: 12px;
        }

        /* =========================
           MAIN WINDOW
        ==========================*/
        QMainWindow {
            background-color: #2a2f33;
        }

        /* =========================
           MENU BAR
        ==========================*/
        QMenuBar {
            background-color: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 #3a3f44,
                stop:1 #2e3338
            );
            border-bottom: 1px solid #1f2327;
            padding: 2px 6px;
        }

        QMenuBar::item {
            background: transparent;
            padding: 6px 10px;
            border-radius: 4px;
        }

        QMenuBar::item:selected {
            background-color: rgba(255, 255, 255, 0.08);
        }

        QMenu {
            background-color: #2e3338;
            border: 1px solid #1f2327;
            padding: 6px 0;
        }

        QMenu::item {
            padding: 6px 14px;
            margin: 2px 8px;
            border-radius: 4px;
        }

        QMenu::item:selected {
            background-color: rgba(255, 255, 255, 0.1);
        }

        /* =========================
           TOOLBARS
        ==========================*/
        QToolBar {
            background-color: #2e3338;
            border: 1px solid #1f2327;
            spacing: 6px;
            padding: 6px;
        }

        QToolBar QToolButton {
            background: transparent;
            border: none;
            border-radius: 4px;
            min-width: 30px;
            min-height: 30px;
            padding: 4px;
        }

        QToolBar QToolButton:hover {
            background-color: rgba(255,255,255,0.08);
        }

        QToolBar QToolButton:checked,
        QToolBar QToolButton:pressed {
            background-color: rgba(0,160,255,0.35);
        }

        QToolBar QLabel {
            background: transparent;
            border: none;
            padding: 0 6px;
            color: #bfc4c9;
        }

        QToolBar QSlider {
            background: transparent;
        }

        QToolBar QWidget {

        }



        QToolButton::menu-indicator {
            image: none;
            width: 0px;
        }

        QStatusBar {
            background: #2a2f33;
            border-top: 1px solid #1f2327;
        }

        QStatusBar QLabel {
            background: transparent;
            padding: 0 6px;
            color: #bfc4c9;
        }

        /* =========================
           PUSH BUTTONS
        ==========================*/
        QPushButton {
            background-color: #3a3f44;
            border: 1px solid #1f2327;
            border-radius: 4px;
            padding: 6px 14px;
        }

        QPushButton:hover {
            background-color: #454b50;
        }

        QPushButton:pressed {
            background-color: #2f73ff;
            border-color: #2f73ff;
        }

        QPushButton:disabled {
            color: #777;
            background-color: #2a2f33;
        }

        /* =========================
           DOCK WIDGETS (PANELS)
        ==========================*/
        QDockWidget {
            background-color: #2e3338;
            border: 1px solid #1f2327;
        }

        QDockWidget::title {
            background-color: #3a3f44;
            padding: 8px 10px;
            font-weight: bold;
            border-bottom: 1px solid #1f2327;
        }

        /* =========================
           LISTS (LAYERS, EFFECTS)
        ==========================*/
        QListWidget {
            background-color: #2a2f33;
            border: none;
            outline: none;
        }

        QListWidget::item {
            padding: 8px 10px;
            margin: 2px 6px;
            border-radius: 4px;
        }

        QListWidget::item:hover {
            background-color: rgba(255,255,255,0.06);
        }

        QListWidget::item:selected {
            background-color: rgba(0,160,255,0.35);
            color: #ffffff;
        }

        /* =========================
           SLIDERS
        ==========================*/
        QSlider::groove:horizontal {
            height: 4px;
            background: rgba(255,255,255,0.2);
            border-radius: 2px;
        }

        QSlider::sub-page:horizontal {
            background: #2f73ff;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            background: #ffffff;
            width: 12px;
            height: 12px;
            border-radius: 6px;   /* 🔥 КЛЮЧОВЕ */
            margin: -4px 0;       /* центрує */
        }

        QSlider::handle:horizontal:hover {
            background: #ffffff;
        }


        /* =========================
           SCROLL BARS
        ==========================*/
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
        }

        QScrollBar::handle:vertical {
            background: rgba(255,255,255,0.15);
            border-radius: 4px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: rgba(255,255,255,0.25);
        }

        QScrollBar::add-line,
        QScrollBar::sub-line {
            background: none;
            height: 0;
        }

        QScrollBar::add-page,
        QScrollBar::sub-page {
            background: none;
        }

        /* =========================
           FORM CONTROLS
        ==========================*/
        QLabel {
            color: #d6d6d6;
        }

        QSpinBox {
            background: transparent;
            border: none;
            border-bottom: 1px solid rgba(255,255,255,0.3);
            padding: 2px 4px;
            min-width: 40px;
        }

        QSpinBox:focus {
            border-bottom: 1px solid #2f73ff;
        }


        QSpinBox::up-button,
        QSpinBox::down-button {
            width: 0;
            height: 0;
        }

        #blendComboBox {
            padding-left: 5px;
            padding-top: 3px;
            padding-bottom: 3px;
        }
    )");
}


void MainWindow::createCentralCanvas()
{
    m_scene = new QGraphicsScene(this);
    m_graphicsView = new MyGraphicsView(m_scene, this);
    m_graphicsView->setLayerManager(&m_layerManager);

    QPixmap checker(40, 40);
    checker.fill(QColor("#1e2226"));

    QPainter p(&checker);
    p.fillRect(0, 0, 20, 20, QColor("#252a2f"));
    p.fillRect(20, 20, 20, 20, QColor("#252a2f"));
    p.end();

    m_graphicsView->setBackgroundBrush(QBrush(checker));
    m_graphicsView->setStyleSheet("QGraphicsView { border: none; }");


    setCentralWidget(m_graphicsView);

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


    m_brushAction = new QAction("Brush", this);
    m_brushAction->setCheckable(true);

    m_eraserAction = new QAction("Eraser", this);
    m_eraserAction->setCheckable(true);

    auto* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(false);
    toolGroup->addAction(m_brushAction);
    toolGroup->addAction(m_eraserAction);

    tb->addSeparator();
    tb->addAction(m_brushAction);
    tb->addAction(m_eraserAction);


    tb->addSeparator();

    tb->addAction(m_undoAction);
    tb->addAction(m_redoAction);

    tb->addSeparator();

    tb->addAction(m_cropAction);


    tb->addSeparator();
    auto rotateLeft = new QAction("RL", this);
    auto rotateRight = new QAction("RR", this);
    auto flipH = new QAction("FH", this);
    auto flipV = new QAction("FV", this);

    tb->addAction(rotateLeft);
    tb->addAction(rotateRight);
    tb->addAction(flipH);
    tb->addAction(flipV);
    QWidget* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet("background: transparent;");
    tb->addWidget(spacer);


    QLabel* zoomLabel = new QLabel("Zoom:", tb);
    zoomLabel->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
    zoomLabel->setFixedHeight(20);

    m_scaleSlider = new QSlider(Qt::Horizontal, tb);
    m_scaleSlider->setFixedHeight(20);
    m_scaleSlider->setRange(10, 400);
    m_scaleSlider->setValue(100);
    m_scaleSlider->setFixedWidth(200);

    auto* zoomLayout = new QHBoxLayout;
    zoomLayout->setContentsMargins(6, 0, 6, 0);
    zoomLayout->setSpacing(8);
    zoomLayout->addWidget(zoomLabel, 0, Qt::AlignVCenter);
    zoomLayout->addWidget(m_scaleSlider, 1, Qt::AlignVCenter);

    auto* zoomWidget = new QWidget(tb);
    zoomWidget->setLayout(zoomLayout);
    zoomWidget->setAttribute(Qt::WA_StyledBackground, false);
    zoomWidget->setStyleSheet("background: transparent;");

    tb->addWidget(zoomWidget);


    tb->addAction(m_zoomInAction);



    connect(rotateLeft, &QAction::triggered, this, [this] {
        int i = m_layerManager.activeLayerIndex();
        undoRedoStack.push(std::make_unique<RotateLayerCommand>(m_layerManager, i, -90));
        updateUndoRedoButtons();
    });

    connect(rotateRight, &QAction::triggered, this, [this] {
        int i = m_layerManager.activeLayerIndex();
        undoRedoStack.push(std::make_unique<RotateLayerCommand>(m_layerManager, i, 90));
        updateUndoRedoButtons();
    });

    connect(flipH, &QAction::triggered, this, [this] {
        int i = m_layerManager.activeLayerIndex();
        undoRedoStack.push(std::make_unique<FlipLayerCommand>(
            m_layerManager, i, FlipLayerCommand::Direction::Horizontal));
        updateUndoRedoButtons();
    });

    connect(flipV, &QAction::triggered, this, [this] {
        int i = m_layerManager.activeLayerIndex();
        undoRedoStack.push(std::make_unique<FlipLayerCommand>(
            m_layerManager, i, FlipLayerCommand::Direction::Vertical));
        updateUndoRedoButtons();
    });

    connect(m_brushAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            m_eraserAction->setChecked(false);
            setActiveTool(m_brushTool.get());
        } else {
            setActiveTool(nullptr);
        }
    });

    connect(m_eraserAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            m_brushAction->setChecked(false);
            setActiveTool(m_eraserTool.get());
        } else {
            setActiveTool(nullptr);
        }
    });




    addToolBar(Qt::TopToolBarArea, tb);


}

void MainWindow::createFilterDock()
{
    m_filterDock = new QDockWidget(tr("Filters"), this);
    m_filterDock->setFeatures(
        QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable
        );

    m_filterDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_filtersPanel = new FiltersPanel(this);

    connect(m_filtersPanel, &FiltersPanel::pipelineChanged,
            this,
            [this](int index, FilterPipeline before, FilterPipeline after)
            {
                auto cmd = std::make_unique<ChangeLayerPipelineCommand>(
                    m_layerManager,
                    index,
                    std::move(before),
                    std::move(after)
                    );
                undoRedoStack.push(std::move(cmd));
                updateUndoRedoButtons();
            });

    connect(m_filtersPanel, &FiltersPanel::previewRequested,
            this, [this]()
            {
                updateComposite();
            });


    m_filterDock->setWidget(m_filtersPanel);
    addDockWidget(Qt::LeftDockWidgetArea, m_filterDock);

    connect(m_filtersPanel, &FiltersPanel::previewRequested,
            this, [this]()
            {
                updateComposite();
            });

}


void MainWindow::createLayersDock()
{
    m_layersDock = new QDockWidget(tr("Layers"), this);
    m_layersDock->setFeatures(
        QDockWidget::DockWidgetMovable |
        QDockWidget::DockWidgetFloatable
        );
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

    connect(m_layersPanel, &LayersPanel::clippedChanged,
            this, [this](int index, bool clipped)
            {
                qDebug() << "UI clippedChanged:" << index << clipped;

                auto cmd = std::make_unique<SetLayerClippedCommand>(
                    m_layerManager,
                    index,
                    clipped
                    );
                undoRedoStack.push(std::move(cmd));
                updateUndoRedoButtons();
            });



    if (m_layersPanel)
        m_layersPanel->setLayers(m_layerManager.layers(), m_layerManager.activeLayerIndex());
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
    m_fitToScreenAction->setShortcut(QKeySequence("Ctrl+0"));
    m_fitToScreenAction->setShortcut(QKeySequence("Ctrl+0"));
    m_saveAction->setShortcut(QKeySequence("Ctrl+S"));
    m_saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    m_brushAction->setShortcut(QKeySequence("B"));
    m_eraserAction->setShortcut(QKeySequence("E"));

}

void MainWindow::initializeTools()
{
    m_brushTool = std::make_unique<BrushTool>(activeLayerImage(), [this]() { updateComposite(); }, m_graphicsView);
    m_eraserTool = std::make_unique<EraserTool>(activeLayerImage(), [this]() { updateComposite(); }, m_graphicsView);

    int size = m_brushSizeSpin ? m_brushSizeSpin->value() : 10;
    m_brushTool->setBrushSize(size);
    m_eraserTool->setBrushSize(size);

    m_brushTool->setColor(m_brushColor);

    setActiveTool(nullptr);
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


void MainWindow::fitToScreen()
{
    if (!m_graphicsView || !m_scaleSlider)
        return;

    auto* item = m_graphicsView->getPixmapItem();
    if (!item)
        return;

    m_graphicsView->resetTransform();

    m_graphicsView->fitInView(item, Qt::KeepAspectRatio);

    const double scale = m_graphicsView->transform().m11();

    int sliderValue = static_cast<int>(scale * 100.0);
    sliderValue = std::clamp(
        sliderValue,
        m_scaleSlider->minimum(),
        m_scaleSlider->maximum()
        );

    m_isUpdatingSlider = true;
    m_scaleSlider->setValue(sliderValue);
    m_isUpdatingSlider = false;
}



QImage* MainWindow::activeLayerImage()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_layerManager.activeLayer());
    if (!layer) return nullptr;

    QImage& img = layer->image();
    Q_ASSERT(img.format() == QImage::Format_ARGB32_Premultiplied);
    return &img;
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

    QImage result = m_layerManager.composite();
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

    selectActiveLayer(insertIndex);

    updateUndoRedoButtons();
}


QImage MainWindow::prepareImageForCanvas(const QImage& source, const QSize& canvasSize) const
{
    QImage converted = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (!canvasSize.isValid())
        return converted;

    if (converted.size() == canvasSize)
        return converted;

    QImage result(canvasSize, QImage::Format_ARGB32_Premultiplied);
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

void MainWindow::selectActiveLayer(int index)
{
    m_layerManager.setActiveLayerIndex(index);

    if (m_filtersPanel)
    {
        m_filtersPanel->setActiveLayer(
            m_layerManager.activeLayer(),
            index
            );
    }

    updateComposite();
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

    auto command = std::make_unique<ReorderLayerCommand>(m_layerManager, from, to);
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
    undoRedoStack.undo();
    updateUndoRedoButtons();
}

void MainWindow::doRedo()
{
    if (!undoRedoStack.canRedo()) return;
    undoRedoStack.redo();
    updateUndoRedoButtons();
}


void MainWindow::updateUndoRedoButtons()
{
    if (m_undoAction) m_undoAction->setEnabled(undoRedoStack.canUndo());
    if (m_redoAction) m_redoAction->setEnabled(undoRedoStack.canRedo());
}

void MainWindow::openImage()
{
    auto imgOpt = ImageIO::openImage(this);
    if (!imgOpt)
        return;

    loadDocument(*imgOpt);
    resetEditorState();
    finalizeDocumentLoad();
}



void MainWindow::save()
{
    QImage result = m_layerManager.composite();
    QImage out = result.convertToFormat(QImage::Format_ARGB32);
    ImageIO::saveImage(this, out, m_currentFilePath);

}

void MainWindow::saveAs()
{
    QImage result = m_layerManager.composite();
    QImage out = result.convertToFormat(QImage::Format_ARGB32);
    ImageIO::saveImageAs(this, out, m_currentFilePath);

}

void MainWindow::loadDocument(const QImage& img)
{
    m_currentFilePath.clear();

    m_layerManager = LayerManager{};
    m_layerManager.setCanvasSize(img.size());

    m_layerManager.setOnChanged([this]() {
        if (m_brushTool)  m_brushTool->setTargetImage(activeLayerImage());
        if (m_eraserTool) m_eraserTool->setTargetImage(activeLayerImage());
        if (m_layersPanel)
            m_layersPanel->setLayers(
                m_layerManager.layers(),
                m_layerManager.activeLayerIndex()
                );
        updateComposite();
    });

    QImage converted = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    auto baseLayer = std::make_shared<PixelLayer>(tr("Background"), converted);
    baseLayer->setOffset(QPointF(0, 0));
    baseLayer->setScale(1.0f);

    m_layerManager.addLayer(baseLayer);
    m_layerManager.setActiveLayerIndex(0);

    if (m_filtersPanel)
    {
        m_filtersPanel->setActiveLayer(
            m_layerManager.activeLayer(),
            m_layerManager.activeLayerIndex()
            );
    }
}


void MainWindow::resetEditorState()
{
    undoRedoStack.clear();
}


void MainWindow::finalizeDocumentLoad()
{

    initializeTools();
    updateUndoRedoButtons();
    fitToScreen();
}

