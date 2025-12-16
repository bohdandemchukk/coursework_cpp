#ifndef STROKECOMMAND_H
#define STROKECOMMAND_H

#include "command.h"
#include <QImage>
#include <QRect>
#include <functional>

class StrokeCommand : public Command
{
public:
    StrokeCommand(QImage* target,
                  const QRect& rect,
                  const QImage& before,
                  const QImage& after,
                  const QImage& mask,
                  std::function<void()> updateCallback);

    void execute() override;
    void undo() override;

private:
    void apply(const QImage& source);

    QImage* m_target {nullptr};
    QRect m_rect {};
    QImage m_before {};
    QImage m_after {};
    QImage m_mask {};
    std::function<void()> m_updateCallback {};
};

#endif // STROKECOMMAND_H
