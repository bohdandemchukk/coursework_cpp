#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QGraphicsScene>
#include <QWheelEvent>
#include "MyGraphicsView.h"
#include <QImage>
#include "filterpipeline.h"

QT_BEGIN_NAMESPACE




namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static QImage originalImage;


private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

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
        bool flipH      { false };
        bool flipV      { false };
        bool BWFilter   { false };
    };

    FilterState filterState{};
    FilterPipeline pipeline{};



private slots:
    void on_actionOpen_triggered();
    void on_actionCrop_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();
    void updateImage();
    void rebuildPipeline();

};

#endif // MAINWINDOW_H
