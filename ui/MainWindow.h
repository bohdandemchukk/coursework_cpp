#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QGraphicsScene>
#include <QDockWidget>
#include <QScrollArea>
#include <QList>
#include <QAction>
#include <QSlider>
#include <QCheckBox>
#include <QString>
#include <QSpinBox>
#include <QPushButton>
#include <QToolButton>
#include <QImage>
#include <QSize>
#include <QWidgetAction>

#include "layers/layermanager.h"
#include "MyGraphicsView.h"
#include "undoredostack.h"
#include "brushtool.h"
#include "erasertool.h"

#include "filterspanel.h"
#include "layerspanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

private:

    QPointF m_compositeOffset;
    QGraphicsScene* m_scene {nullptr};
    MyGraphicsView* m_graphicsView {nullptr};

    QMenuBar* m_menuBar {nullptr};
    QToolBar* m_toolBar {nullptr};

    QDockWidget* m_filterDock {nullptr};
    QWidget* m_filterPanel {nullptr};

    FiltersPanel* m_filtersPanel = nullptr;


    QDockWidget* m_layersDock {nullptr};
    LayersPanel* m_layersPanel {nullptr};


    QSlider* m_scaleSlider {nullptr};


    QAction* m_openAction {nullptr};
    QAction* m_saveAction {nullptr};
    QAction* m_saveAsAction {nullptr};
    QAction* m_exitAction {nullptr};

    QAction* m_undoAction {nullptr};
    QAction* m_redoAction {nullptr};

    QAction* m_cropAction {nullptr};

    QAction* m_panAction {nullptr};
    QAction* m_fitToScreenAction {nullptr};
    QWidgetAction* m_zoomInAction {nullptr};
    QAction* m_zoomOutAction {nullptr};
    QAction* m_brushAction  {nullptr};
    QAction* m_eraserAction {nullptr};


    QString m_currentFilePath{};

    LayerManager m_layerManager{};
    UndoRedoStack undoRedoStack;


    bool m_isUpdatingSlider {false};
    bool m_isPanToolActive  {false};
    bool m_isSpacePanActive {false};

    std::unique_ptr<BrushTool> m_brushTool;
    std::unique_ptr<EraserTool> m_eraserTool;
    Tool* m_activeTool {nullptr};
    QToolButton* m_toolsButton {nullptr};
    QSpinBox* m_brushSizeSpin {nullptr};
    QPushButton* m_colorButton {nullptr};
    QColor m_brushColor {Qt::white};

    void applyGlobalStyle();
    void createActions();
    void createTopBar();
    void createCentralCanvas();
    void createFilterDock();
    void createLayersDock();
    void setupShortcuts();
    void initializeTools();
    void setActiveTool(Tool* tool);
    void updateBrushColorButton();

    void updateComposite();
    void updateActiveLayerImage(const QImage &image);
    QImage* activeLayerImage();
    void selectActiveLayer(int index);
    void updateUndoRedoButtons();

    void handleAddLayer();
    void handleAddAdjustmentLayer();
    void handleAddImageLayer();
    void handleDeleteLayer(int managerIndex);
    void handleMoveLayer(int from, int to);
    void handleVisibilityChanged(int managerIndex, bool visible);
    void handleOpacityChanged(int managerIndex, float opacity);

    void loadDocument(const QImage& img);
    void resetEditorState();
    void finalizeDocumentLoad();


    QImage prepareImageForCanvas(const QImage& source, const QSize& canvasSize) const;

private slots:

    void openImage();
    void save();
    void saveAs();
    void doUndo();
    void doRedo();
    void fitToScreen();
    void updatePanMode();
};

#endif // MAINWINDOW_H
