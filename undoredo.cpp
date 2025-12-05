#include "undoredo.h"

void CommandManager::executeCommand(std::unique_ptr<Command> command)
{
    if (!command) return;

    command->execute();
    m_undoStack.push_back(std::move(command));
    m_redoStack.clear();
}

bool CommandManager::undo()
{
    if (m_undoStack.empty()) return false;

    auto command {std::move(m_undoStack.back())};
    m_undoStack.pop_back();
    command->undo();
    m_redoStack.push_back(std::move(command));
    return true;
}

bool CommandManager::redo()
{
    if (m_redoStack.empty()) return false;

    auto command {std::move(m_redoStack.back())};
    m_redoStack.pop_back();
    command->execute();
    m_undoStack.push_back(std::move(command));
    return true;
}

void CommandManager::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}
