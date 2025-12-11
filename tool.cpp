#include "tool.h"

Tool::Tool(QImage* targetImage, std::function<void()> updateCallback)
    : m_targetImage(targetImage),
      m_updateCallback(std::move(updateCallback))
{}

void Tool::setBrushSize(int size)
{
    m_brushSize = std::max(1, size);
}

void Tool::setColor(const QColor& color)
{
    m_color = color;
}

void Tool::requestUpdate() const
{
    if (m_updateCallback) {
        m_updateCallback();
    }
}
