#ifndef CHANGEFILTERBOOLCOMMAND_H
#define CHANGEFILTERBOOLCOMMAND_H

#include <functional>

#include "command.h"

class ChangeFilterBoolCommand: public Command
{
public:
    ChangeFilterBoolCommand(bool* target, bool oldValue, bool newValue, std::function<void()> rebuildCallback);

    void execute() override;

    void undo() override;

private:
    bool* m_target;
    bool m_oldValue;
    bool m_newValue;
    std::function<void()> m_rebuildCallback;
};

#endif // CHANGEFILTERBOOLCOMMAND_H
