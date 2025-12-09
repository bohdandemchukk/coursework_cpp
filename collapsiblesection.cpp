#include "CollapsibleSection.h"
#include <QLabel>

CollapsibleSection::CollapsibleSection(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_expanded(false)
    , m_contentHeight(0)
{
    m_toggleButton = new QPushButton(this);
    m_toggleButton->setText("▶ " + title);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setStyleSheet(R"(
        QPushButton {
            background-color: #2b2b2b;
            color: #cccccc;
            border: none;
            text-align: left;
            padding: 10px 15px;
            font-weight: bold;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #3a3a3a;
        }
        QPushButton:checked {
            background-color: #3a3a3a;
        }
    )");


    m_contentArea = new QFrame(this);
    m_contentArea->setStyleSheet(R"(
        QFrame {
            background-color: #252525;
            border: none;
        }
    )");
    m_contentArea->setMaximumHeight(0);
    m_contentArea->setMinimumHeight(0);


    m_animation = new QPropertyAnimation(m_contentArea, "maximumHeight", this);
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);


    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_toggleButton);
    mainLayout->addWidget(m_contentArea);

    connect(m_toggleButton, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            m_toggleButton->setText(m_toggleButton->text().replace("▶", "▼"));
            m_animation->setStartValue(0);
            m_animation->setEndValue(m_contentHeight);
            m_animation->start();
            m_expanded = true;
        } else {
            m_toggleButton->setText(m_toggleButton->text().replace("▼", "▶"));
            m_animation->setStartValue(m_contentHeight);
            m_animation->setEndValue(0);
            m_animation->start();
            m_expanded = false;
        }
        emit toggled(m_expanded);
    });
}

void CollapsibleSection::setContentLayout(QLayout *contentLayout)
{
    delete m_contentArea->layout();
    m_contentArea->setLayout(contentLayout);


    m_contentHeight = contentLayout->sizeHint().height();
}

void CollapsibleSection::toggle()
{
    m_toggleButton->setChecked(!m_toggleButton->isChecked());
}
