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

MoveLayerCommand::MoveLayerCommand(LayerManager &manager, int from, int to)
    : m_manager(manager), m_from(from), m_to(to)
{
}

void MoveLayerCommand::execute()
{
    if (m_manager.moveLayer(m_from, m_to))
    {
        std::swap(m_from, m_to);
    }
}

void MoveLayerCommand::undo()
{
    // execute already swapped indices for undo
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
