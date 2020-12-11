#include "MBloomWork.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MTextureRenderTarget.h"

M_OBJECT_IMPLEMENT(MBloomWork, MStandardPostProcessWork)

MBloomWork::MBloomWork()
    : MStandardPostProcessWork()
{
}

MBloomWork::~MBloomWork()
{
}

void MBloomWork::Initialize(MIRenderProgram* pRenderProgram)
{
    Super::Initialize(pRenderProgram);

    InitializeMaterial();
}

void MBloomWork::Release()
{
    ReleaseMaterial();

    Super::Release();
}

void MBloomWork::Render(MPostProcessRenderInfo& info)
{
    info.pRenderer->BeginRenderPass(m_pTempRenderPass, m_pTempRenderTarget);

    Vector2 v2Size = info.pViewport->GetSize();
    info.pRenderer->SetViewport(0, 0, v2Size.x, v2Size.y, 0, 1);

    if (info.pRenderer->SetUseMaterial(m_pMaterial))
    {
        info.pRenderer->DrawMesh(m_pScreenDrawMesh);
    }


    info.pRenderer->EndRenderPass();
}

void MBloomWork::InitializeMaterial()
{

}

void MBloomWork::ReleaseMaterial()
{

}

