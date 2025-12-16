#include "changelayerpipelinecommand.h"
#include "layers/layer.h"
#include <QDebug>

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
    qDebug() << "ChangeLayerPipelineCommand::execute() - index:" << m_index;
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(
        m_manager.layerAt(m_index)
        );
    if (!layer) {
        qDebug() << "Layer not found or not adjustment!";
        return;
    }

    qDebug() << "Setting pipeline to 'after' state";
    layer->pipeline() = m_after;
    m_manager.notifyLayerChanged();
}

void ChangeLayerPipelineCommand::undo()
{
    qDebug() << "ChangeLayerPipelineCommand::undo() - index:" << m_index;
    auto layer = std::dynamic_pointer_cast<AdjustmentLayer>(
        m_manager.layerAt(m_index)
        );
    if (!layer) {
        qDebug() << "Layer not found or not adjustment!";
        return;
    }

    qDebug() << "Setting pipeline to 'before' state";
    layer->pipeline() = m_before;
    m_manager.notifyLayerChanged();
}
