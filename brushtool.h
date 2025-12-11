#ifndef BRUSHTOOL_H
#define BRUSHTOOL_H

#include "tool.h"
#include "strokecommand.h"
#include "MyGraphicsView.h"

class BrushTool : public Tool
{
public:
    explicit BrushTool(QImage* targetImage, std::function<void()> updateCallback, MyGraphicsView* view);

    void onMousePress(const QPoint& imagePos, Qt::MouseButton button) override;
    void onMouseMove(const QPoint& imagePos, Qt::MouseButtons buttons) override;
    std::unique_ptr<Command> onMouseRelease(const QPoint& imagePos, Qt::MouseButton button) override;

protected:
    virtual void paintStroke(QPainter& painter, const QPoint& from, const QPoint& to);

private:
    void beginStroke(const QPoint& pos);
    void continueStroke(const QPoint& pos);
    std::unique_ptr<Command> endStroke();
    QRect expandedRect(const QPoint& a, const QPoint& b) const;
    bool m_drawing {false};
    QPoint m_lastPos {};
    QRect m_boundingRect {};
    QImage m_mask {};
    QImage m_preStrokeSnapshot {};
};

#endif // BRUSHTOOL_H
