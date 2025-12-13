#include "layercommands.h"

AddLayerCommand::AddLayerCommand(LayerManager &manager, std::shared_ptr<Layer> layer, int index)
    : m_manager(manager), m_layer(std::move(layer)), m_requestedIndex(index)
{
}

void AddLayerCommand::execute()
{
    m_insertedIndex = m_manager.addLayer(m_layer, m_requestedIndex);
}

void AddLayerCommand::undo()
{
    if (m_insertedIndex >= 0)
    {
        m_manager.removeLayer(m_insertedIndex);
    }
}

RemoveLayerCommand::RemoveLayerCommand(LayerManager &manager, int index)
    : m_manager(manager), m_index(index)
{
}

void RemoveLayerCommand::execute()
{
    m_removedLayer = m_manager.removeLayer(m_index);
}

void RemoveLayerCommand::undo()
{
    if (m_removedLayer)
    {
        m_manager.addLayer(m_removedLayer, m_index);
    }
}

ReorderLayerCommand::ReorderLayerCommand(LayerManager &manager, int from, int to)
    : m_manager(manager), m_from(from), m_to(to)
{
}

void ReorderLayerCommand::execute()
{
    if (m_manager.moveLayer(m_from, m_to))
    {
        std::swap(m_from, m_to);
    }
}
void ReorderLayerCommand::undo()
{
    m_manager.moveLayer(m_from, m_to);
    std::swap(m_from, m_to);
}

SetLayerVisibilityCommand::SetLayerVisibilityCommand(LayerManager &manager, int index, bool visible)
    : m_manager(manager), m_index(index), m_newVisible(visible)
{
}

void SetLayerVisibilityCommand::execute()
{
    auto layer = m_manager.layerAt(m_index);
    if (!layer)
        return;

    m_oldVisible = layer->isVisible();
    layer->setVisible(m_newVisible);
    m_manager.notifyLayerChanged();
}

void SetLayerVisibilityCommand::undo()
{
    auto layer = m_manager.layerAt(m_index);
    if (!layer)
        return;

    layer->setVisible(m_oldVisible);
    m_manager.notifyLayerChanged();
}

SetLayerOpacityCommand::SetLayerOpacityCommand(LayerManager &manager, int index, float opacity)
    : m_manager(manager), m_index(index), m_newOpacity(opacity)
{
}

void SetLayerOpacityCommand::execute()
{
    auto layer = m_manager.layerAt(m_index);
    if (!layer)
        return;

    m_oldOpacity = layer->opacity();
    layer->setOpacity(m_newOpacity);
    m_manager.notifyLayerChanged();
}

void SetLayerOpacityCommand::undo()
{
    auto layer = m_manager.layerAt(m_index);
    if (!layer)
        return;

    layer->setOpacity(m_oldOpacity);
    m_manager.notifyLayerChanged();
}

SetLayerBlendModeCommand::SetLayerBlendModeCommand(LayerManager& manager,
                         int index,
                         BlendMode newMode)
    : m_manager(manager), m_index(index), m_new(newMode)
{}

void SetLayerBlendModeCommand::execute()
{
    auto layer = m_manager.layerAt(m_index);
    if (!layer) return;

    m_old = layer->blendMode();
    layer->setBlendMode(m_new);
    m_manager.notifyLayerChanged();
}

void SetLayerBlendModeCommand::undo()
{
    auto layer = m_manager.layerAt(m_index);
    if (!layer) return;

    layer->setBlendMode(m_old);
    m_manager.notifyLayerChanged();
}

MoveLayerCommand::MoveLayerCommand(LayerManager& manager, int index, QPointF oldOffset, QPointF newOffset)
    : m_manager(manager), m_index(index), m_oldOffset(oldOffset), m_newOffset(newOffset)
{}

void MoveLayerCommand::execute()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_manager.layerAt(m_index));
    if (!layer)
        return;

    layer->setOffset(m_newOffset);
    m_manager.notifyLayerChanged();
}

void MoveLayerCommand::undo()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_manager.layerAt(m_index));
    if (!layer)
        return;

    layer->setOffset(m_oldOffset);
    m_manager.notifyLayerChanged();
}

ScaleLayerCommand::ScaleLayerCommand(LayerManager& manager, int index, float oldScale, float newScale)
    : m_manager(manager), m_index(index), m_oldScale(oldScale), m_newScale(newScale)
{}

void ScaleLayerCommand::execute()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_manager.layerAt(m_index));
    if (!layer)
        return;

    layer->setScale(m_newScale);
    m_manager.notifyLayerChanged();
}

void ScaleLayerCommand::undo()
{
    auto layer = std::dynamic_pointer_cast<PixelLayer>(m_manager.layerAt(m_index));
    if (!layer)
        return;

    layer->setScale(m_oldScale);
    m_manager.notifyLayerChanged();
}



ChangeLayerPipelineCommand::ChangeLayerPipelineCommand(
    LayerManager& manager,
    int layerIndex,
    FilterPipeline before,
    FilterPipeline after
    )
    : m_manager(manager),
    m_index(layerIndex),
    m_before(std::move(before)),
    m_after(std::move(after))
{}

void ChangeLayerPipelineCommand::execute()
{
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(m_manager.layerAt(m_index));
    if (!layer) return;

    layer->pipeline() = m_after;
    m_manager.notifyLayerChanged();
}

void ChangeLayerPipelineCommand::undo()
{
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(m_manager.layerAt(m_index));
    if (!layer) return;

    layer->pipeline() = m_before;
    m_manager.notifyLayerChanged();
}

SetLayerClippedCommand::SetLayerClippedCommand(LayerManager& mgr, int index, bool clipped)
        : m_mgr(mgr), m_index(index), m_new(clipped)
    {
        m_old = mgr.layerAt(index)->isClipped();
    }

void SetLayerClippedCommand::undo() {
        m_mgr.layerAt(m_index)->setClipped(m_old);
        m_mgr.notifyLayerChanged();
    }

void SetLayerClippedCommand::execute() {
        m_mgr.layerAt(m_index)->setClipped(m_new);
        m_mgr.notifyLayerChanged();
    }
