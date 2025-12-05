#ifndef UNDOREDOSTACK_H
#define UNDOREDOSTACK_H

#include <memory>
#include "command.h"

class UndoRedoStack
{
public:
    UndoRedoStack() = default;

    void push(std::unique_ptr<Command> command);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void clear();

private:
    std::vector<std::unique_ptr<Command>> m_commands;
    int m_currentIndex = -1;
    const int MAX_HISTORY = 100;
};

#endif // UNDOREDOSTACK_H
