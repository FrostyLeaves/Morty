#pragma once
#include "Utility/MRenderGlobal.h"

namespace morty
{

struct IPipelineCmd;
class MRenderCommandVulkan;

class MORTY_API MVulkanCommandExecuteTable
{
public:
    explicit MVulkanCommandExecuteTable();

    void RunCommand(MRenderCommandVulkan* executer, const IPipelineCmd* command);

    void PerProcess(MRenderCommandVulkan* executer, const IPipelineCmd* command);

private:
    std::vector<std::function<void(MRenderCommandVulkan*, const IPipelineCmd*)>> m_commandFunction;

    std::vector<std::function<void(MRenderCommandVulkan*, const IPipelineCmd*)>> m_perProcessFunction;
};

}// namespace morty