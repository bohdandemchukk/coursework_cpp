#include "rotatelayercommand.h"
#include "layers/layer.h"
#include <QTransform>

RotateLayerCommand::RotateLayerCommand(LayerManager& mgr, int index, int angle)
    : m_mgr(mgr), m_index(index), m_angle(angle) {}

void RotateLayerCommand::execute()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_mgr.layerAt(m_index));
    if (!layer) return;

    m_before = layer->image();

    QTransform t;
    t.rotate(m_angle);
    layer->setImage(layer->image().transformed(t));

    m_mgr.notifyLayerChanged();
}

void RotateLayerCommand::undo()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_mgr.layerAt(m_index));
    if (!layer) return;

    layer->setImage(m_before);
    m_mgr.notifyLayerChanged();
}
