#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QGraphicsScene>
#include <QWheelEvent>
#include "mygraphicsview.h"
#include <QImage>

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


    QPixmap m_originalImage{};

    QPixmap getOriginalImage() {
        return m_originalImage;
    }

    void setOriginalImage(QPixmap img) {
        m_originalImage = img;
    }

    static QImage originalImage;

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    struct ImageState {
        int brightness  { 0 };
        int saturation  { 0 };
        int rotateAngle { 0 };
        bool flipH      { false };
        bool flipV      { false };
    };

    ImageState imageState {};





private slots:
    void on_actionOpen_triggered();
    void on_actionCrop_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();

    void updateImage();

};

#endif // MAINWINDOW_H
