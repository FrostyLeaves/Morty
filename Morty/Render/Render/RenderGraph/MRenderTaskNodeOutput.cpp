#include "MRenderTaskNodeOutput.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeOutput, MTaskNodeOutput)

void MRenderTaskNodeOutput::SetRenderTarget(MRenderTaskTarget* pRenderTarget) { m_renderTaskTarget = pRenderTarget; }

MTexturePtr MRenderTaskNodeOutput::GetTexture() const
{
    if (m_renderTaskTarget) { return m_renderTaskTarget->GetTexture(); }

    return nullptr;
}
