#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPointer>
#include <QMap>
#include <QToolBar>
#include <QDockWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include "MyGraphicsView.h"
#include "filterpipeline.h"
#include "undoredostack.h"
#include "changefilterintcommand.h"
#include "changefilterboolcommand.h"

class AccordionSection : public QWidget
{
    Q_OBJECT
public:
    explicit AccordionSection(const QString &title, QWidget *parent = nullptr);

    QVBoxLayout *contentLayout() const { return m_contentLayout; }
    void toggle();
    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }

signals:
    void expanded(AccordionSection *section);

private:
    void animateContent(int start, int end);

    QPushButton *m_headerButton { nullptr };
    QWidget *m_content { nullptr };
    QVBoxLayout *m_contentLayout { nullptr };
    QPropertyAnimation *m_animation { nullptr };
    bool m_expanded { false };
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QImage originalImage;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    // UI helpers
    void setupUi();
    void createMenus();
    void createToolbar();
    QWidget *createCentralView();
    void createFilterDock();
    QSlider *addSlider(AccordionSection *section, const QString &name, int min, int max, int defaultValue);
    void applyDarkPalette();

    // Filter helpers
    void changeFilterInt(int* target, int newValue);
    void changeFilterBool(bool* target, bool newValue);
    void rebuildPipeline();
    void updateUndoRedoButtons();
    void updateImage();
    void resetFilterControls();
    void updateZoomDisplay(double scale);

    void setupShortcuts();
    void updatePanMode();

private slots:
    void openImage();
    void saveImage();
    void onCropFinished(const QRect& rect);
    void rotateLeft();
    void rotateRight();
    void flipHorizontally();
    void flipVertically();
    void fitToScreen();
    void togglePanTool();
    void undo();
    void redo();

private:
    QGraphicsScene *scene { nullptr };
    MyGraphicsView *graphicsView { nullptr };
    QDockWidget *filterDock { nullptr };

    QAction *openAction { nullptr };
    QAction *saveAction { nullptr };
    QAction *undoAction { nullptr };
    QAction *redoAction { nullptr };
    QAction *cropAction { nullptr };
    QAction *rotateLeftAction { nullptr };
    QAction *rotateRightAction { nullptr };
    QAction *flipHAction { nullptr };
    QAction *flipVAction { nullptr };
    QAction *fitToScreenAction { nullptr };
    QAction *panToolAction { nullptr };
    QAction *brushAction { nullptr };
    QAction *eraserAction { nullptr };

    QLabel *zoomLabel { nullptr };

    struct FilterState {
        int brightness  { 0 };
        int saturation  { 0 };
        int contrast    { 0 };
        int rotateAngle { 0 };
        int blur        { 0 };
        int sharpness   { 0 };
        int temperature { 0 };
        int exposure    { 0 };
        int gamma       { 0 };
        int tint        { 0 };
        int vibrance    { 0 };
        int shadow      { 0 };
        int highlight   { 0 };
        int clarity     { 0 };
        int vignette    { 0 };
        int grain       { 0 };
        int splitToning { 0 };
        int fade        { 0 };
        bool flipH      { false };
        bool flipV      { false };
        bool BWFilter   { false };
    };

    FilterState filterState{};
    FilterPipeline pipeline{};
    UndoRedoStack undoRedoStack{};

    bool m_isUpdatingSlider {false};
    bool m_isPanToolActive {false};
    bool m_isSpacePanActive {false};

    QMap<QString, QSlider*> m_sliders;
    QPushButton *bwButton { nullptr };
};

#endif // MAINWINDOW_H
