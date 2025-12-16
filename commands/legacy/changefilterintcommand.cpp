#include "changefilterintcommand.h"

ChangeFilterIntCommand::ChangeFilterIntCommand(int* target, int oldValue, int newValue, std::function<void()> rebuildCallback)
    : m_target(target)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_rebuildCallback(rebuildCallback)
{}


void ChangeFilterIntCommand::execute() {
    *m_target = m_newValue;
    m_rebuildCallback();
}

void ChangeFilterIntCommand::undo() {
    *m_target = m_oldValue;
    m_rebuildCallback();
}
