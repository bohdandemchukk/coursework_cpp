#include "fliplayercommand.h"
#include "layer.h"

FlipLayerCommand::FlipLayerCommand(
    LayerManager& mgr, int index, Direction dir
    )
    : m_mgr(mgr), m_index(index), m_dir(dir) {}

void FlipLayerCommand::execute()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_mgr.layerAt(m_index));
    if (!layer) return;

    m_before = layer->image();

    layer->setImage(
        layer->image().mirrored(
            m_dir == Direction::Horizontal,
            m_dir == Direction::Vertical
            )
        );

    m_mgr.notifyLayerChanged();
}

void FlipLayerCommand::undo()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_mgr.layerAt(m_index));
    if (!layer) return;

    layer->setImage(m_before);
    m_mgr.notifyLayerChanged();
}
