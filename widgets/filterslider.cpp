#include "FilterSlider.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>


FilterSlider::FilterSlider(const QString& name, const QIcon& icon, int min, int max, int defaultValue, QWidget *parent)
    : QWidget(parent), m_defaultValue(defaultValue)
{


    setObjectName("FilterSlider");
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setFixedSize(16, 16);
    iconLabel->setPixmap(
        QIcon(icon).pixmap(16, 16)
        );
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel* label = new QLabel(name, this);
    label->setFixedWidth(70);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    setAttribute(Qt::WA_StyledBackground, true);



    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimumWidth(120);
    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_spinBox = new QSpinBox(this);
    resetButton = new QPushButton(QIcon(":/icons/toolbar/undo"),"", this);

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

    auto* sliderRow = new QHBoxLayout;
    sliderRow->setSpacing(6);
    sliderRow->setContentsMargins(0, 0, 0, 0);

    m_spinBox->setFixedWidth(48);

    sliderRow->addWidget(m_slider, 1);
    sliderRow->addWidget(m_spinBox, 0);


    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* labelRow = new QHBoxLayout;
    labelRow->setSpacing(6);
    labelRow->setContentsMargins(0, 0, 0, 0);

    labelRow->addWidget(iconLabel);
    labelRow->addWidget(label);

    layout->addLayout(labelRow);
    layout->addLayout(sliderRow, 1);
    layout->addWidget(resetButton);

    setLayout(layout);

    connect(m_slider, &QSlider::sliderPressed, this, [this]() {
        emit sliderPressed();
    });

    connect(m_slider, &QSlider::valueChanged, m_spinBox, &QSpinBox::setValue);
    connect(m_spinBox, &QSpinBox::valueChanged, m_slider, &QSlider::setValue);

    connect(m_slider, &QSlider::sliderReleased, this, [this]() {
        emit sliderReleased(m_slider->value());
    });

    connect(m_slider, &QSlider::valueChanged, this, [this]() {
        emit valueChanged(m_slider->value());
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


