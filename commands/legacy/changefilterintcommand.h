#ifndef CHANGEFILTERINTCOMMAND_H
#define CHANGEFILTERINTCOMMAND_H

#include <functional>
#include <qdebug.h>
#include "command.h"

class ChangeFilterIntCommand: public Command
{
public:
    ChangeFilterIntCommand(int* target, int oldValue, int newValue, std::function<void()> rebuildCallback);

    void execute() override;

    void undo() override;

private:
    int* m_target;
    int m_oldValue;
    int m_newValue;
    std::function<void()> m_rebuildCallback;
};

#endif // CHANGEFILTERINTCOMMAND_H
