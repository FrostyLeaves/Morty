/**
 * @File         MConsoleSystem
 *
 * @Created      2026-01-08
 *
 * @Author       DoubleYe
 **/

#pragma once

#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Utility/MString.h"

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <map>
#include <vector>

namespace morty
{

class MORTY_API MConsoleSystem : public MISystem
{
    MORTY_CLASS(MConsoleSystem)

public:
    using CommandFunc = std::function<bool(MEngine*, const std::vector<MString>&)>;

    MConsoleSystem();
    virtual ~MConsoleSystem();

    void Initialize() override;
    void Release() override;
    void EngineTick(const float& delta) override;

    void RegisterCommand(const MString& name, const MString& desc, CommandFunc func);
    void UnregisterCommand(const MString& name);

private:
    void                 ConsoleThreadFunc();
    void                 ProcessCommand(const MString& line);
    std::vector<MString> ParseArgs(const MString& line);
    void                 RegisterBuiltinCommands();

    struct CommandInfo
    {
        MString     description;
        CommandFunc func;
    };
    std::map<MString, CommandInfo> m_commands;

    std::queue<MString>  m_commandQueue;
    std::mutex           m_queueMutex;

    std::thread          m_consoleThread;
    std::atomic<bool>    m_running{false};
    std::atomic<bool>    m_shouldStop{false};
};

}// namespace morty
