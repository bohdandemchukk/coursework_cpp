#include "cropcommand.h"

CropCommand::CropCommand(LayerManager& mgr, const QRect& cropRect)
    : m_mgr(mgr),
    m_cropRect(cropRect)
{
    m_oldCanvasSize = mgr.canvasSize();
    m_newCanvasSize = cropRect.size();

    for (int i = 0; i < mgr.layerCount(); ++i) {
        auto pixel = std::dynamic_pointer_cast<PixelLayer>(mgr.layerAt(i));
        if (!pixel) continue;

        LayerState s;
        s.layer = pixel;
        s.before = pixel->image();
        s.oldOffset = pixel->offset();
        m_layers.push_back(std::move(s));
    }
}


void CropCommand::execute()
{
    m_mgr.setCanvasSize(m_newCanvasSize);

    for (auto& s : m_layers) {
        QRect imgRect(
            int((m_cropRect.left() - s.oldOffset.x()) / s.layer->scale()),
            int((m_cropRect.top()  - s.oldOffset.y()) / s.layer->scale()),
            int(m_cropRect.width()  / s.layer->scale()),
            int(m_cropRect.height() / s.layer->scale())
            );

        imgRect = imgRect.intersected(s.layer->image().rect());

        if (!imgRect.isEmpty()) {
            QImage cropped = s.layer->image().copy(imgRect);
            s.layer->setImage(cropped);
        }

        s.layer->setOffset(QPointF(0, 0));
    }

    m_mgr.notifyLayerChanged();
}


void CropCommand::undo()
{
    m_mgr.setCanvasSize(m_oldCanvasSize);

    for (auto& s : m_layers) {
        s.layer->setImage(s.before);
        s.layer->setOffset(s.oldOffset);
    }

    m_mgr.notifyLayerChanged();
}
