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
#include <QPixmap>
#include <memory>

#include <QSvgRenderer>
#include "changelayerpipelinecommand.h"
#include "rotatelayercommand.h"
#include "fliplayercommand.h"
#include "imageio.h"
#include "cropcommand.h"
#include "layercommands.h"
#include <QPainter>
#include <QPixmap>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
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

    m_undoAction = new QAction(QIcon(":/icons/toolbar/undo.svg"), "", this);
    m_undoAction->setToolTip("Undo");
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::doUndo);

    m_redoAction = new QAction(QIcon(":/icons/toolbar/redo.svg"), "", this);
    m_redoAction->setToolTip("Redo");
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::doRedo);

    m_cropAction = new QAction(QIcon(":/icons/toolbar/crop.svg"), "", this);
    m_cropAction->setToolTip("Crop");
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

    tb->setStyleSheet(R"(
QToolBar {
    spacing: 6px;
    padding: 4px;
}

QToolButton {
    background: transparent;
    border: none;
    padding: 0px 8px;
    min-height: 32px;
    max-height: 32px;
}

QToolButton:hover {
    background: rgba(255,255,255,0.08);
}

QToolButton:pressed {
    background: rgba(255,255,255,0.15);
}
QToolBar::separator {
    background: rgba(255,255,255,0.18);
    width: 1px;
    height: 22px;
    margin-left: 8px;
    margin-right: 8px;
    margin-top: 5px;
    margin-bottom: 5px;
}


)");


    tb->setIconSize(QSize(20, 20));
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);

    auto setupAction = [&](QAction* action) {
        auto btn = qobject_cast<QToolButton*>(tb->widgetForAction(action));
        if (!btn) return;

        btn->setFixedSize(32, 32);
        btn->setIconSize(QSize(20, 20));
        btn->setAutoRaise(true);
    };

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


    m_brushAction = new QAction(QIcon(":/icons/toolbar/brush.svg"), "", this);
    m_brushAction->setCheckable(true);

    m_eraserAction = new QAction(QIcon(":/icons/toolbar/eraser.svg"), "", this);
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
    auto rotateLeft  = new QAction(QIcon(":/icons/toolbar/rleft.svg"), "", this);
    auto rotateRight = new QAction(QIcon(":/icons/toolbar/rright.svg"), "", this);
    auto flipH       = new QAction(QIcon(":/icons/toolbar/fliph.svg"), "", this);
    auto flipV       = new QAction(QIcon(":/icons/toolbar/flipv.svg"), "", this);

    rotateLeft->setToolTip("Rotate Left");
    rotateRight->setToolTip("Rotate Right");
    flipH->setToolTip("Flip Horizontal");
    flipV->setToolTip("Flip Vertical");
    m_brushAction->setToolTip("Brush");
    m_eraserAction->setToolTip("Eraser");

    tb->addAction(rotateLeft);
    tb->addAction(rotateRight);
    tb->addAction(flipH);
    tb->addAction(flipV);
    QWidget* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setStyleSheet("background: transparent;");
    tb->addWidget(spacer);

    setupAction(rotateLeft);
    setupAction(rotateRight);
    setupAction(flipH);
    setupAction(flipV);
    setupAction(m_brushAction);
    setupAction(m_eraserAction);


    QLabel* zoomLabel = new QLabel;
    zoomLabel->setPixmap(
        QIcon(":/icons/toolbar/zoom.svg")
            .pixmap(16, 16)
        );
    zoomLabel->setFixedSize(16, 16);
    zoomLabel->setAlignment(Qt::AlignCenter);
    zoomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);


    m_scaleSlider = new QSlider(Qt::Horizontal);
    m_scaleSlider->setRange(10, 400);
    m_scaleSlider->setValue(100);
    m_scaleSlider->setFixedWidth(180);
    m_scaleSlider->setFixedHeight(18);
    m_scaleSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_scaleSlider->setStyleSheet("margin-right: 5px; margin-left: 10px;");




    QHBoxLayout* zoomLayout = new QHBoxLayout;
    zoomLayout->setContentsMargins(0, 0, 0, 0);
    zoomLayout->setSpacing(0);
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(m_scaleSlider);



    QWidget* zoomWidget = new QWidget;
    zoomWidget->setStyleSheet("background: transparent; ");
    zoomWidget->setLayout(zoomLayout);
    zoomWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);


    QWidgetAction* m_zoomInAction = new QWidgetAction(tb);
    m_zoomInAction->setDefaultWidget(zoomWidget);

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


    auto* scrollArea = new QScrollArea(this);  
    scrollArea->setWidget(m_filtersPanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);


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


    m_filterDock->setWidget(scrollArea);

    addDockWidget(Qt::LeftDockWidgetArea, m_filterDock);
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



    m_graphicsView->resetTransform();
    m_graphicsView->setTransform(QTransform());



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



    if (result.isNull() || m_layerManager.layerCount() == 0)
    {
        m_graphicsView->clearPixmap();
    }
    else
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

    if (m_filtersPanel) {
        m_filtersPanel->setActiveLayer(
            m_layerManager.activeLayer(),
            m_layerManager.layerCount() - 1
            );
    }

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

    if (m_filtersPanel) {
        m_filtersPanel->setActiveLayer(
            m_layerManager.activeLayer(),
            insertIndex
            );
    }

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

    if (m_filtersPanel) {
        m_filtersPanel->setActiveLayer(
            m_layerManager.activeLayer(),
            m_layerManager.activeLayerIndex()
            );
    }
}


void MainWindow::doRedo()
{
    if (!undoRedoStack.canRedo()) return;

    undoRedoStack.redo();
    updateUndoRedoButtons();

    if (m_filtersPanel) {
        m_filtersPanel->setActiveLayer(
            m_layerManager.activeLayer(),
            m_layerManager.activeLayerIndex()
            );
    }
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

