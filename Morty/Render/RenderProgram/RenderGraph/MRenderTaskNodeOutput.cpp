#include "MRenderTaskNodeOutput.h"

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeOutput, MTaskNodeOutput)

void MRenderTaskNodeOutput::SetRenderTarget(MRenderTaskTarget* pRenderTarget)
{
    m_pRenderTaskTarget = pRenderTarget;
}

std::shared_ptr<MTexture> MRenderTaskNodeOutput::GetTexture() const
{
    if (m_pRenderTaskTarget)
    {
        return m_pRenderTaskTarget->GetTexture();
    }

    return nullptr;
}
