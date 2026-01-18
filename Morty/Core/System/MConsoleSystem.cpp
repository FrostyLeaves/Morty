#include "System/MConsoleSystem.h"

#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"

#include <iostream>
#include <sstream>

using namespace morty;

MORTY_CLASS_IMPLEMENT(MConsoleSystem, MISystem)

MConsoleSystem::MConsoleSystem()
    : MISystem()
{}

MConsoleSystem::~MConsoleSystem() {}

void             MConsoleSystem::Initialize()
{
    RegisterBuiltinCommands();
#ifdef MORTY_RESOURCE_PATH
    RegisterUserCommandsFromConfig(MString(MORTY_RESOURCE_PATH) + "/Config/ConsoleCommands.yml");
#endif

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

void MConsoleSystem::RegisterUserCommand(const MString& name, const MString& desc, const std::vector<MString>& commands)
{
    if (name.empty()) { return; }

    RegisterCommand(
            name,
            desc,
            [this, commands](MEngine* engine, const std::vector<MString>& args) -> bool {
                MORTY_UNUSED(engine);
                MORTY_UNUSED(args);
                for (const auto& line: commands)
                {
                    if (!line.empty()) { ProcessCommand(line); }
                }
                return true;
            }
    );
}

void MConsoleSystem::RegisterUserCommandsFromConfig(const MString& filePath)
{
    if (filePath.empty()) { return; }

    YAML::Node root;
    try {
        root = YAML::LoadFile(filePath);
    } catch (const YAML::BadFile&) {
        return;
    } catch (const YAML::Exception& e) {
        std::cout << "Failed to load console command config: " << filePath << " (" << e.what() << ")"
                  << std::endl;
        return;
    }

    const YAML::Node commandsNode = root["commands"];
    if (!commandsNode || !commandsNode.IsSequence()) { return; }

    for (const auto& commandNode: commandsNode)
    {
        const YAML::Node nameNode = commandNode["name"];
        if (!nameNode || !nameNode.IsScalar()) { continue; }

        const MString name = nameNode.as<MString>();
        const MString desc = commandNode["description"] ? commandNode["description"].as<MString>() : "User command";

        std::vector<MString> commandLines;
        const YAML::Node     linesNode = commandNode["commands"];
        if (linesNode)
        {
            if (linesNode.IsScalar())
            {
                commandLines.push_back(linesNode.as<MString>());
            }
            else if (linesNode.IsSequence())
            {
                for (const auto& lineNode: linesNode)
                {
                    if (lineNode.IsScalar()) { commandLines.push_back(lineNode.as<MString>()); }
                }
            }
        }

        if (!commandLines.empty()) { RegisterUserCommand(name, desc, commandLines); }
    }
}

void MConsoleSystem::UnregisterCommand(const MString& name) { m_commands.erase(name); }

void MConsoleSystem::ExecuteCommand(const MString& line)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_commandQueue.push(MString(line));
}


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
    std::vector<MString> tokens;
    std::istringstream   stream(line);
    std::string          token;

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

    RegisterCommand("quit", "Request engine shutdown", [](MEngine* engine, const std::vector<MString>& args) -> bool {
        MORTY_UNUSED(args);
        if (engine) { engine->Stop(); }
        return true;
    });

    RegisterCommand("exit", "Request engine shutdown", [](MEngine* engine, const std::vector<MString>& args) -> bool {
        MORTY_UNUSED(args);
        if (engine) { engine->Stop(); }
        return true;
    });
}
