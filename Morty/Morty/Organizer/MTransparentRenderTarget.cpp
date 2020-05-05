#include "MTransparentRenderTarget.h"
#include "MEngine.h"

MTypeIdentifierImplement(MTransparentRenderTarget, MObject)

MTransparentRenderTarget::MTransparentRenderTarget()
    : MObject()
    , m_nCurLevel(0)
    , m_nLevelNumber(4)
    , m_pTransparentMeshes(nullptr)
{
}

MTransparentRenderTarget::~MTransparentRenderTarget()
{
}

void MTransparentRenderTarget::OnCreated()
{
	m_pDevice = m_pEngine->GetDevice();
}

void MTransparentRenderTarget::OnDelete()
{

}

void MTransparentRenderTarget::OnRender(MIRenderer* pRenderer)
{

}

