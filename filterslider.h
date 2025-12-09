#ifndef FILTERSLIDER_H
#define FILTERSLIDER_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QSpinBox>
#include <QPushButton>

class FilterSlider : public QWidget
{
    Q_OBJECT
public:
    explicit FilterSlider(const QString &name, int min, int max, int defaultValue, QWidget *parent = nullptr);


    int m_defaultValue{};

    int getValue() const;



    void setValue(int value);

signals:
    void sliderReleased(int value);

private:
    QPushButton* resetButton{};
    QLabel *m_nameLabel{};
    QSlider *m_slider{};
    QSpinBox *m_spinBox{};
};

#endif
