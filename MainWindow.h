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


#include "MyGraphicsView.h"
#include "filterpipeline.h"
#include "undoredostack.h"
#include "changefilterintcommand.h"
#include "changefilterboolcommand.h"
#include "brushtool.h"
#include "erasertool.h"

#include "CollapsibleSection.h"
#include "FilterSlider.h"


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

    QGraphicsScene* m_scene {nullptr};
    MyGraphicsView* m_graphicsView {nullptr};

    QMenuBar* m_menuBar {nullptr};
    QToolBar* m_toolBar {nullptr};

    QDockWidget* m_filterDock {nullptr};
    QWidget* m_filterPanel {nullptr};
    QList<CollapsibleSection*> m_sections{};

    QSlider* m_scaleSlider {nullptr};

    FilterSlider* m_exposureSlider    {nullptr};
    FilterSlider* m_contrastSlider    {nullptr};
    FilterSlider* m_brightnessSlider  {nullptr};
    FilterSlider* m_highlightSlider   {nullptr};
    FilterSlider* m_shadowSlider      {nullptr};
    FilterSlider* m_claritySlider     {nullptr};

    FilterSlider* m_temperatureSlider {nullptr};
    FilterSlider* m_tintSlider        {nullptr};
    FilterSlider* m_saturationSlider  {nullptr};
    FilterSlider* m_vibranceSlider    {nullptr};

    FilterSlider* m_splitToningSlider {nullptr};
    FilterSlider* m_vignetteSlider    {nullptr};
    FilterSlider* m_grainSlider       {nullptr};
    FilterSlider* m_fadeSlider        {nullptr};

    FilterSlider* m_sharpenSlider {nullptr};
    FilterSlider* m_blurSlider    {nullptr};
    FilterSlider* m_gammaSlider   {nullptr};

    QCheckBox* m_bwCheckbox {nullptr};


    QAction* m_openAction {nullptr};
    QAction* m_saveAction {nullptr};
    QAction* m_saveAsAction {nullptr};
    QAction* m_exitAction {nullptr};

    QAction* m_undoAction {nullptr};
    QAction* m_redoAction {nullptr};

    QAction* m_cropAction {nullptr};
    QAction* m_rotateLeftAction {nullptr};
    QAction* m_rotateRightAction {nullptr};
    QAction* m_flipHAction {nullptr};
    QAction* m_flipVAction {nullptr};

    QAction* m_panAction {nullptr};
    QAction* m_fitToScreenAction {nullptr};
    QAction* m_zoomInAction {nullptr};
    QAction* m_zoomOutAction {nullptr};


    QString m_currentFilePath{};

    struct FilterState {
        int brightness  {0};
        int saturation  {0};
        int contrast    {0};
        int rotateAngle {0};
        int blur        {0};
        int sharpness   {0};
        int temperature {0};
        int exposure    {0};
        int gamma       {0};
        int tint        {0};
        int vibrance    {0};
        int shadow      {0};
        int highlight   {0};
        int clarity     {0};
        int vignette    {0};
        int grain       {0};
        int splitToning {0};
        int fade        {0};

        bool flipH      {false};
        bool flipV      {false};
        bool BWFilter   {false};
    };

    FilterState filterState;
    FilterPipeline pipeline;
    UndoRedoStack undoRedoStack;

    QImage workingImage {};

    bool m_isUpdatingSlider {false};
    bool m_isPanToolActive  {false};
    bool m_isSpacePanActive {false};

    std::unique_ptr<BrushTool> m_brushTool;
    std::unique_ptr<EraserTool> m_eraserTool;
    Tool* m_activeTool {nullptr};
    CollapsibleSection* m_toolsSection {nullptr};
    QToolButton* m_toolsButton {nullptr};
    QSpinBox* m_brushSizeSpin {nullptr};
    QPushButton* m_colorButton {nullptr};
    QColor m_brushColor {Qt::white};

    void applyGlobalStyle();
    void createActions();
    void createTopBar();
    void createCentralCanvas();
    void createFilterDock();
    void createFilterSections();
    void setupShortcuts();
    void createToolsSection(QVBoxLayout* layout);
    void initializeTools();
    void setActiveTool(Tool* tool);
    void updateBrushColorButton();

    void updateImage();
    void rebuildPipeline();
    void updateUndoRedoButtons();

    void changeFilterInt(int* target, int newValue);
    void changeFilterBool(bool* target, bool newValue);

private slots:

    void openImage();
    void save();
    void saveAs();
    void doUndo();
    void doRedo();
    void onCropFinished(const QRect& rect);
    void rotateLeft();
    void rotateRight();
    void flipH();
    void flipV();
    void fitToScreen();
    void updatePanMode();
};

#endif // MAINWINDOW_H
