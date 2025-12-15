#ifndef LAYERCOMMANDS_H
#define LAYERCOMMANDS_H

#include <memory>

#include "command.h"
#include <QPointF>
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

class ReorderLayerCommand : public Command
{
public:
    ReorderLayerCommand(LayerManager &manager, int from, int to);

    void execute() override;
    void undo() override;

private:
    LayerManager &m_manager;
    int m_from;
    int m_to;
};

class MoveLayerCommand : public Command
{
public:
    MoveLayerCommand(LayerManager& manager, int index, QPointF oldOffset, QPointF newOffset);

    void execute() override;
    void undo() override;

private:
    LayerManager& m_manager;
    int m_index;
    QPointF m_oldOffset;
    QPointF m_newOffset;
};

class ScaleLayerCommand : public Command
{
public:
    ScaleLayerCommand(LayerManager& manager, int index, float oldScale, float newScale);

    void execute() override;
    void undo() override;

private:
    LayerManager& m_manager;
    int m_index;
    float m_oldScale;
    float m_newScale;
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

class SetLayerBlendModeCommand : public Command
{
public:
    SetLayerBlendModeCommand(LayerManager& manager, int index, BlendMode newMode);

    void execute() override;
    void undo() override;


private:
    LayerManager& m_manager;
    int m_index;
    BlendMode m_old {BlendMode::Normal};
    BlendMode m_new {BlendMode::Normal};
};

#endif // LAYERCOMMANDS_H



class SetLayerClippedCommand : public Command {
public:
    SetLayerClippedCommand(LayerManager& mgr, int index, bool clipped);



    void undo() override;

    void execute() override;

private:
    LayerManager& m_mgr;
    int m_index;
    bool m_old, m_new;
};


class TransformLayerCommand : public Command
{
public:
    TransformLayerCommand(LayerManager& manager,
                          int index,
                          QPointF oldOffset, float oldScale,
                          QPointF newOffset, float newScale);

    void execute() override;
    void undo() override;

private:
    LayerManager& m_manager;
    int m_index;
    QPointF m_oldOffset, m_newOffset;
    float m_oldScale, m_newScale;
};
