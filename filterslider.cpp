#include "FilterSlider.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

FilterSlider::FilterSlider(const QString &name, int min, int max, int defaultValue, QWidget *parent)
    : QWidget(parent), m_defaultValue(defaultValue)
{
    QLabel* label = new QLabel(name, this);

    label->setFixedWidth(70);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_spinBox = new QSpinBox(this);
    resetButton = new QPushButton("↺", this);

    m_slider->setRange(min, max);
    m_slider->setValue(m_defaultValue);

    m_spinBox->setRange(min, max);
    m_spinBox->setValue(m_defaultValue);

    resetButton->setFixedWidth(26);
    resetButton->setToolTip("Reset to default");
    resetButton->setStyleSheet(
        "QPushButton { background-color: #333; color: #ddd; border-radius: 4px; }"
        "QPushButton:hover { background-color: #444; }"
        );

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(m_slider);
    layout->addWidget(m_spinBox);
    layout->addWidget(resetButton);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    connect(m_slider, &QSlider::valueChanged, m_spinBox, &QSpinBox::setValue);
    connect(m_spinBox, &QSpinBox::valueChanged, m_slider, &QSlider::setValue);

    connect(m_slider, &QSlider::sliderReleased, this, [this]() {
        emit sliderReleased(m_slider->value());
    });

    connect(resetButton, &QPushButton::clicked, this, [this]() {
        m_slider->setValue(m_defaultValue);
        m_spinBox->setValue(m_defaultValue);
        emit sliderReleased(m_defaultValue);
    });
}


int FilterSlider::getValue() const
{
    return m_slider->value();
}

void FilterSlider::setValue(int value)
{
    m_slider->setValue(value);
}
