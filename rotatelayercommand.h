#ifndef ROTATELAYERCOMMAND_H
#define ROTATELAYERCOMMAND_H

#include "command.h"
#include "layermanager.h"

class RotateLayerCommand : public Command {
public:
    RotateLayerCommand(LayerManager& mgr, int index, int angle);

    void execute() override;
    void undo() override;

private:
    LayerManager& m_mgr;
    int m_index;
    int m_angle;
    QImage m_before;
};

#endif // ROTATELAYERCOMMAND_H
