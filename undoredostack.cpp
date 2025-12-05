#include <vector>
#include "undoredostack.h"


void UndoRedoStack::push(std::unique_ptr<Command> command) {

    if (m_currentIndex < static_cast<int>(m_commands.size()) - 1) {
        m_commands.erase(m_commands.begin() + m_currentIndex + 1, m_commands.end());
    }

    command->execute();
    m_commands.push_back(std::move(command));
    m_currentIndex = m_commands.size() - 1;


    if (m_commands.size() > MAX_HISTORY) {
        m_commands.erase(m_commands.begin());
        m_currentIndex--;
    }
}

bool UndoRedoStack::canUndo() const {
    return m_currentIndex >= 0;
}

bool UndoRedoStack::canRedo() const {
    return m_currentIndex < static_cast<int>(m_commands.size()) - 1;
}

void UndoRedoStack::undo() {
    if (canUndo()) {
        m_commands[m_currentIndex]->undo();
        m_currentIndex--;
    }
 }

void UndoRedoStack::redo() {
    if (canRedo()) {
        m_currentIndex++;
        m_commands[m_currentIndex]->execute();
    }
}

void UndoRedoStack::clear() {
    m_commands.clear();
    m_currentIndex = -1;
}
