#include <vector>
#include "undoredostack.h"
#include <QDebug>


void UndoRedoStack::push(std::unique_ptr<Command> command) {
    qDebug() << "=== PUSH COMMAND ===";
    qDebug() << "Current index:" << m_currentIndex;
    qDebug() << "Stack size before:" << m_commands.size();

    if (m_currentIndex < static_cast<int>(m_commands.size()) - 1) {
        qDebug() << "Erasing future commands from" << (m_currentIndex + 1);
        m_commands.erase(m_commands.begin() + m_currentIndex + 1, m_commands.end());
    }

    command->execute();
    m_commands.push_back(std::move(command));
    m_currentIndex = m_commands.size() - 1;

    qDebug() << "Stack size after:" << m_commands.size();
    qDebug() << "New index:" << m_currentIndex;
}

void UndoRedoStack::undo() {
    qDebug() << "=== UNDO ===";
    qDebug() << "Current index:" << m_currentIndex;
    qDebug() << "Stack size:" << m_commands.size();

    if (canUndo()) {
        qDebug() << "Executing undo at index:" << m_currentIndex;
        m_commands[m_currentIndex]->undo();
        m_currentIndex--;
        qDebug() << "New index:" << m_currentIndex;
    } else {
        qDebug() << "Cannot undo!";
    }
}

void UndoRedoStack::redo() {
    qDebug() << "=== REDO ===";
    qDebug() << "Current index:" << m_currentIndex;
    qDebug() << "Stack size:" << m_commands.size();

    if (canRedo()) {
        m_currentIndex++;
        qDebug() << "Executing redo at index:" << m_currentIndex;
        m_commands[m_currentIndex]->execute();
        qDebug() << "New index:" << m_currentIndex;
    } else {
        qDebug() << "Cannot redo!";
    }
}

bool UndoRedoStack::canUndo() const {
    return m_currentIndex >= 0;
}

bool UndoRedoStack::canRedo() const {
    return m_currentIndex < static_cast<int>(m_commands.size()) - 1;
}



void UndoRedoStack::clear() {
    m_commands.clear();
    m_currentIndex = -1;
}
