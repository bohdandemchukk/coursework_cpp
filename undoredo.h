#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <functional>
#include <memory>
#include <vector>

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

template <typename T>
class PropertyCommand : public Command
{
public:
    PropertyCommand(T& target, T newValue, std::function<void()> onApplied)
        : m_target{target}
        , m_oldValue{target}
        , m_newValue{newValue}
        , m_onApplied{std::move(onApplied)}
    {
    }

    void execute() override
    {
        m_target = m_newValue;
        if (m_onApplied) m_onApplied();
    }

    void undo() override
    {
        m_target = m_oldValue;
        if (m_onApplied) m_onApplied();
    }

private:
    T& m_target;
    T m_oldValue;
    T m_newValue;
    std::function<void()> m_onApplied;
};

class CommandManager
{
public:
    void executeCommand(std::unique_ptr<Command> command);
    bool undo();
    bool redo();
    void clear();

    [[nodiscard]] bool canUndo() const { return !m_undoStack.empty(); }
    [[nodiscard]] bool canRedo() const { return !m_redoStack.empty(); }

private:
    std::vector<std::unique_ptr<Command>> m_undoStack{};
    std::vector<std::unique_ptr<Command>> m_redoStack{};
};

#endif // UNDOREDO_H
