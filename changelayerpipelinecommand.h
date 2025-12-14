#ifndef CHANGELAYERPIPELINECOMMAND_H
#define CHANGELAYERPIPELINECOMMAND_H
#include "layer.h"
#include "command.h"
#include "layermanager.h"

class ChangeLayerPipelineCommand : public Command {
public:
    ChangeLayerPipelineCommand(
        LayerManager& manager,
        int index,
        FilterPipeline before,
        FilterPipeline after
        );

    void undo() override;
    void execute() override;

private:
    LayerManager& m_manager;
    int m_index;
    FilterPipeline m_before;
    FilterPipeline m_after;
};

#endif // CHANGELAYERPIPELINECOMMAND_H
