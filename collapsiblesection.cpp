#include "CollapsibleSection.h"
#include <QLabel>
#include <QStyle>


CollapsibleSection::CollapsibleSection(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_expanded(false)
    , m_contentHeight(0)
{
    setObjectName("CollapsibleSection");
    setAttribute(Qt::WA_StyledBackground, true);
    m_toggleButton = new QPushButton(this);
    m_toggleButton->setText(title);
    m_toggleButton->setObjectName("SectionHeader");

    m_toggleButton->setCheckable(true);
    m_toggleButton->setIcon(QIcon(":/icons/filters/chevronright.svg"));
    m_toggleButton->setIconSize(QSize(12, 12));




    m_contentArea = new QFrame(this);
    m_contentArea->setObjectName("SectionContent");
    m_contentArea->setAttribute(Qt::WA_StyledBackground, true);


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
            m_toggleButton->setIcon(QIcon(":/icons/filters/chevrondown.svg"));
            m_animation->setStartValue(0);
            m_animation->setEndValue(m_contentHeight);
            m_animation->start();
            m_expanded = true;
        } else {
            m_toggleButton->setIcon(QIcon(":/icons/filters/chevronright.svg"));
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


