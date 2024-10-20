#include "MRenderTaskNodeOutput.h"
#include "MRenderTaskNode.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeOutput, MTaskNodeOutput)

void MRenderTaskNodeOutput::SetRenderTarget(MRenderTaskTarget* pRenderTarget) { m_renderTaskTarget = pRenderTarget; }

MTexturePtr MRenderTaskNodeOutput::GetTexture() const
{
    if (m_renderTaskTarget) { return m_renderTaskTarget->GetTexture(); }

    return nullptr;
}
MRenderTaskOutputDesc MRenderTaskNodeOutput::GetOutputDesc() const
{
    const size_t nIndex = GetIndex();
    return GetTaskNode()->DynamicCast<MRenderTaskNode>()->InitOutputDesc().at(nIndex);
}
