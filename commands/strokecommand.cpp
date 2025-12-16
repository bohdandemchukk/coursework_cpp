#include "strokecommand.h"

StrokeCommand::StrokeCommand(QImage* target,
                             const QRect& rect,
                             const QImage& before,
                             const QImage& after,
                             const QImage& mask,
                             std::function<void()> updateCallback)
    : m_target(target),
    m_rect(rect),
    m_before(before),
    m_after(after),
    m_mask(mask),
    m_updateCallback(std::move(updateCallback))
{}

void StrokeCommand::apply(const QImage& source)
{
    if (!m_target || m_rect.isEmpty()) return;

    for (int y = 0; y < m_rect.height(); ++y) {
        const uchar* maskLine = m_mask.constScanLine(y);
        const QRgb* srcLine = reinterpret_cast<const QRgb*>(source.constScanLine(y));
        QRgb* dstLine = reinterpret_cast<QRgb*>(m_target->scanLine(m_rect.top() + y)) + m_rect.left();

        for (int x = 0; x < m_rect.width(); ++x) {
            if (maskLine[x]) {
                dstLine[x] = srcLine[x];
            }
        }
    }

    if (m_updateCallback) {
        m_updateCallback();
    }
}

void StrokeCommand::undo()
{
    apply(m_before);
    if (m_updateCallback)
        m_updateCallback();
}


void StrokeCommand::execute()
{
    apply(m_after);
    if (m_updateCallback)
        m_updateCallback();
}
