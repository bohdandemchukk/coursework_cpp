#include "changelayerpipelinecommand.h"
#include "layer.h"

ChangeLayerPipelineCommand::ChangeLayerPipelineCommand(
    LayerManager& manager,
    int index,
    FilterPipeline before,
    FilterPipeline after
    )
    : m_manager(manager)
    , m_index(index)
    , m_before(std::move(before))
    , m_after(std::move(after))
{}

void ChangeLayerPipelineCommand::execute()
{
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(
        m_manager.layerAt(m_index)
        );
    if (!layer) return;

    layer->pipeline() = m_after;
    m_manager.notifyLayerChanged();
}

void ChangeLayerPipelineCommand::undo()
{
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(
        m_manager.layerAt(m_index)
        );
    if (!layer) return;

    layer->pipeline() = m_before;
    m_manager.notifyLayerChanged();
}
