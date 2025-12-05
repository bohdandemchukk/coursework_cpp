#include "changefilterboolcommand.h"

ChangeFilterBoolCommand::ChangeFilterBoolCommand(bool* target, bool oldValue, bool newValue, std::function<void()> rebuildCallback)
    : m_target(target)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_rebuildCallback(rebuildCallback)
{}


void ChangeFilterBoolCommand::execute() {
    *m_target = m_newValue;
    m_rebuildCallback();
}

void ChangeFilterBoolCommand::undo() {
    *m_target = m_oldValue;
    m_rebuildCallback();
}
