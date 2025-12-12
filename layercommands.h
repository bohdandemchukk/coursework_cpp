#ifndef LAYERCOMMANDS_H
#define LAYERCOMMANDS_H

#include <memory>

#include "command.h"
#include "layermanager.h"

class AddLayerCommand : public Command
{
public:
    AddLayerCommand(LayerManager &manager, std::shared_ptr<Layer> layer, int index = -1);

    void execute() override;
    void undo() override;

private:
    LayerManager &m_manager;
    std::shared_ptr<Layer> m_layer;
    int m_requestedIndex;
    int m_insertedIndex = -1;
};

class RemoveLayerCommand : public Command
{
public:
    RemoveLayerCommand(LayerManager &manager, int index);

    void execute() override;
    void undo() override;

private:
    LayerManager &m_manager;
    int m_index;
    std::shared_ptr<Layer> m_removedLayer;
};

class MoveLayerCommand : public Command
{
public:
    MoveLayerCommand(LayerManager &manager, int from, int to);

    void execute() override;
    void undo() override;

private:
    LayerManager &m_manager;
    int m_from;
    int m_to;
};

class SetLayerVisibilityCommand : public Command
{
public:
    SetLayerVisibilityCommand(LayerManager &manager, int index, bool visible);

    void execute() override;
    void undo() override;

private:
    LayerManager &m_manager;
    int m_index;
    bool m_newVisible;
    bool m_oldVisible {false};
};

class SetLayerOpacityCommand : public Command
{
public:
    SetLayerOpacityCommand(LayerManager &manager, int index, float opacity);

    void execute() override;
    void undo() override;

private:
    LayerManager &m_manager;
    int m_index;
    float m_newOpacity;
    float m_oldOpacity {1.0f};
};

#endif // LAYERCOMMANDS_H
