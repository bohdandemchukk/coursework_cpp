#ifndef FLIPLAYERCOMMAND_H
#define FLIPLAYERCOMMAND_H

#include "command.h"
#include "layermanager.h"

class FlipLayerCommand : public Command {
public:
    enum class Direction { Horizontal, Vertical };

    FlipLayerCommand(LayerManager& mgr, int index, Direction dir);

    void execute() override;
    void undo() override;

private:
    LayerManager& m_mgr;
    int m_index;
    Direction m_dir;
    QImage m_before;
};

#endif // FLIPLAYERCOMMAND_H
