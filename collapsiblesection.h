#ifndef COLLAPSIBLESECTION_H
#define COLLAPSIBLESECTION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QPropertyAnimation>

class CollapsibleSection : public QWidget
{
    Q_OBJECT
public:
    explicit CollapsibleSection(const QString &title, QWidget *parent = nullptr);

    void setContentLayout(QLayout *contentLayout);
    void toggle();
    bool isExpanded() const { return m_expanded; }



signals:
    void toggled(bool expanded);

private:
    QPushButton *m_toggleButton{};
    QFrame *m_contentArea{};
    QPropertyAnimation *m_animation{};
    bool m_expanded{};
    int m_contentHeight{};
};

#endif
