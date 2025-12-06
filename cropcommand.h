#ifndef CROPCOMMAND_H
#define CROPCOMMAND_H

#include "command.h"
#include <QImage>
#include <QRect>
#include <functional>

class CropCommand: public Command
{
public:
    CropCommand(QImage* target, QRect newRect, std::function<void()> rebuildCallback);
    void undo() override;
    void execute() override;

private:
    QImage* m_target{};
    QImage m_oldValue{};
    QRect m_newValue{};
    std::function<void()> m_rebuildCallback{};
};

#endif // CROPCOMMAND_H
