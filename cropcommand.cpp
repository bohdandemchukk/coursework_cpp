#include "cropcommand.h"

CropCommand::CropCommand(QImage* target, QRect newRect, std::function<void()> rebuildCallback)
    : m_target {target},
    m_oldValue {*target},
    m_newValue {newRect},
    m_rebuildCallback {rebuildCallback}
{}


void CropCommand::execute() {
    *m_target = m_oldValue.copy(m_newValue);
    m_rebuildCallback();
}

void CropCommand::undo() {
    *m_target = m_oldValue;
    m_rebuildCallback();
}
