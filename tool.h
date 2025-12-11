#ifndef TOOL_H
#define TOOL_H

#include <QColor>
#include <QMouseEvent>
#include <QPoint>
#include <QImage>
#include <functional>
#include "MyGraphicsView.h"

class Command;

class Tool
{
public:
    explicit Tool(QImage* targetImage, std::function<void()> updateCallback, MyGraphicsView* view);
    virtual ~Tool() = default;

    virtual void onMousePress(const QPoint& imagePos, Qt::MouseButton button) = 0;
    virtual void onMouseMove(const QPoint& imagePos, Qt::MouseButtons buttons) = 0;
    virtual std::unique_ptr<Command> onMouseRelease(const QPoint& imagePos, Qt::MouseButton button) = 0;

    void setBrushSize(int size);
    int brushSize() const { return m_brushSize; }

    void setColor(const QColor& color);
    QColor color() const { return m_color; }

protected:
    QImage* targetImage() const { return m_targetImage; }
    MyGraphicsView* m_view{};
    void requestUpdate() const;

private:
    int m_brushSize {10};

    QColor m_color {Qt::white};
    QImage* m_targetImage {nullptr};
    std::function<void()> m_updateCallback {};
};

#endif // TOOL_H
