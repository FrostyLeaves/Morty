//
// Created by yerenming on 2024/11/9.
//

#include "MVulkanCommandExecuteTable.h"
#include "MRenderCommandVulkan.h"

using namespace morty;

morty::MVulkanCommandExecuteTable::MVulkanCommandExecuteTable()
{
    size_t maxCommandNum = 10;
    m_commandFunction.resize(maxCommandNum);
    m_perProcessFunction.resize(maxCommandNum);

    m_commandFunction[MSetViewportCmd::GetType()] = [](MRenderCommandVulkan* self, const IPipelineCmd* cmd) {
        self->SetViewport((const MSetViewportCmd*) cmd);
    };

    m_commandFunction[MSetScissorCmd::GetType()] = [](auto* self, auto* cmd) {
        self->SetScissor((const MSetScissorCmd*) cmd);
    };

    m_commandFunction[MDrawMeshCmd::GetType()] = [](auto* self, auto* cmd) {
        self->DrawMesh((const MDrawMeshCmd*) cmd);
    };

    m_commandFunction[MDrawIndexedIndirectCmd::GetType()] = [](auto* self, auto* cmd) {
        self->DrawIndexedIndirect((const MDrawIndexedIndirectCmd*) cmd);
    };

    m_commandFunction[MSetGraphPipelineCmd::GetType()] = [](auto* self, auto* cmd) {
        self->SetGraphPipeline((const MSetGraphPipelineCmd*) cmd);
    };

    m_commandFunction[MSetShaderParameterSetCmd::GetType()] = [](auto* self, auto* cmd) {
        self->SetShaderParameterSet((const MSetShaderParameterSetCmd*) cmd);
    };

    m_commandFunction[MNextSubPassCmd::GetType()] = [](auto* self, auto* cmd) {
        self->NextSubPass((const MNextSubPassCmd*) cmd);
    };

    m_commandFunction[MSetShadingRateCmd::GetType()] = [](auto* self, auto* cmd) {
        self->SetShadingRate((const MSetShadingRateCmd*) cmd);
    };


    m_perProcessFunction[MSetShaderParameterSetCmd::GetType()] = [](auto* self, auto* cmd) {
        self->AddBarrierForPixelSample((const MSetShaderParameterSetCmd*) cmd);
    };
}

void MVulkanCommandExecuteTable::RunCommand(MRenderCommandVulkan* executer, const IPipelineCmd* command)
{
    MORTY_ASSERT(m_commandFunction[command->type]);

    m_commandFunction[command->type](executer, command);
}

void MVulkanCommandExecuteTable::PerProcess(MRenderCommandVulkan* executer, const IPipelineCmd* command)
{
    if (m_perProcessFunction[command->type]) { m_perProcessFunction[command->type](executer, command); }
}
