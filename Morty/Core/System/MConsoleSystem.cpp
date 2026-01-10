#include "System/MConsoleSystem.h"

#include "Engine/MEngine.h"
#include "Utility/MGlobal.h"

#include <iostream>
#include <sstream>

using namespace morty;

MORTY_CLASS_IMPLEMENT(MConsoleSystem, MISystem)

MConsoleSystem::MConsoleSystem()
    : MISystem()
{}

MConsoleSystem::~MConsoleSystem() {}

void MConsoleSystem::Initialize()
{
    RegisterBuiltinCommands();

    m_shouldStop.store(false);
    m_running.store(true);
    m_consoleThread = std::thread(&MConsoleSystem::ConsoleThreadFunc, this);
}

void MConsoleSystem::Release()
{
    m_shouldStop.store(true);

    if (m_consoleThread.joinable())
    {
        // Detach thread since std::getline blocks and cannot be interrupted on Windows
        m_consoleThread.detach();
    }

    m_running.store(false);
    m_commands.clear();
}

void MConsoleSystem::EngineTick(const float& delta)
{
    MORTY_UNUSED(delta);

    std::queue<MString> localQueue;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::swap(localQueue, m_commandQueue);
    }

    while (!localQueue.empty())
    {
        ProcessCommand(localQueue.front());
        localQueue.pop();
    }
}

void MConsoleSystem::RegisterCommand(const MString& name, const MString& desc, CommandFunc func)
{
    m_commands[name] = CommandInfo{desc, func};
}

void MConsoleSystem::UnregisterCommand(const MString& name) { m_commands.erase(name); }

void MConsoleSystem::ConsoleThreadFunc()
{
    std::string line;
    while (!m_shouldStop.load())
    {
        if (std::getline(std::cin, line))
        {
            if (!line.empty())
            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                m_commandQueue.push(MString(line));
            }
        }
        else
        {
            // EOF or error, exit the loop
            break;
        }
    }
    m_running.store(false);
}

void MConsoleSystem::ProcessCommand(const MString& line)
{
    std::vector<MString> args = ParseArgs(line);
    if (args.empty()) { return; }

    const MString& cmdName = args[0];
    auto           it      = m_commands.find(cmdName);
    if (it != m_commands.end())
    {
        bool success = it->second.func(GetEngine(), args);
        if (!success) { std::cout << "Command failed: " << cmdName << std::endl; }
    }
    else { std::cout << "Unknown command: " << cmdName << std::endl; }
}

std::vector<MString> MConsoleSystem::ParseArgs(const MString& line)
{
    std::vector<MString>  tokens;
    std::istringstream    stream(line);
    std::string           token;

    while (stream >> token) { tokens.push_back(MString(token)); }

    return tokens;
}

void MConsoleSystem::RegisterBuiltinCommands()
{
    RegisterCommand(
            "help",
            "List all available commands",
            [this](MEngine* engine, const std::vector<MString>& args) -> bool {
                MORTY_UNUSED(engine);
                MORTY_UNUSED(args);
                std::cout << "Available commands:" << std::endl;
                for (const auto& [name, info]: m_commands)
                {
                    std::cout << "  " << name << " - " << info.description << std::endl;
                }
                return true;
            }
    );

    RegisterCommand(
            "quit",
            "Request engine shutdown",
            [](MEngine* engine, const std::vector<MString>& args) -> bool {
                MORTY_UNUSED(args);
                if (engine) { engine->Stop(); }
                return true;
            }
    );

    RegisterCommand(
            "exit",
            "Request engine shutdown",
            [](MEngine* engine, const std::vector<MString>& args) -> bool {
                MORTY_UNUSED(args);
                if (engine) { engine->Stop(); }
                return true;
            }
    );
}
