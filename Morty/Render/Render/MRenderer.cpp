#include "MRenderer.h"
#include "RHI/Command/MRenderPassCmd.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(IRenderer, MTypeClass)

void MIndexedIndirectRenderer::Execute(MRenderPassCmd* primaryCommand) const
{
    if (indirectBuffer == nullptr || vertexBuffer == nullptr || indexBuffer == nullptr || instanceCount == 0)
    {
        return;
    }

    primaryCommand->DrawIndexedIndirect(indexBuffer, vertexBuffer, indirectBuffer, indirectOffset, instanceCount);
}

void MIndexedIndirectCountRenderer::Execute(MRenderPassCmd* primaryCommand) const
{
    if (indirectBuffer == nullptr || vertexBuffer == nullptr || indexBuffer == nullptr || countBuffer == nullptr)
    {
        return;
    }

    for (const auto& drawCall: drawCalls)
    {
        primaryCommand->SetGraphPipeline(drawCall.pass);
        primaryCommand->ApplyPushedShaderParameterSets();
        primaryCommand->SetShaderParameterSet(drawCall.parameterSet);

        primaryCommand->DrawIndexedIndirectCount(
                vertexBuffer,
                indexBuffer,
                indirectBuffer,
                countBuffer,
                drawCall.commandOffset,
                drawCall.countOffset,
                drawCall.maxCount
        );
    }
}