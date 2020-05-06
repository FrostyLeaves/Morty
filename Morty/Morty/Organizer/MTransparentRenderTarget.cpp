#include "MTransparentRenderTarget.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MTexture.h"

MTypeIdentifierImplement(MTransparentRenderTarget, MObject)

MTransparentRenderTarget::MTransparentRenderTarget()
    : MObject()
    , m_Material(nullptr)
    , m_pSceneDepthTexture(nullptr)
    , m_pPrevLevelRenderTarget(nullptr)
    , m_pTransparentMeshes(nullptr)
{
}

MTransparentRenderTarget::~MTransparentRenderTarget()
{
}

void MTransparentRenderTarget::OnCreated()
{
	m_pDevice = m_pEngine->GetDevice();

	MMaterial* pMaterial = m_Material.GetResource<MMaterial>();
    pMaterial->SetRasterizerType(MERasterizerType::ECullNone);

}

void MTransparentRenderTarget::OnDelete()
{
    m_Material.SetResource(nullptr);
}

void MTransparentRenderTarget::OnRender(MIRenderer* pRenderer)
{
    MMaterial* pMaterial = m_Material.GetResource<MMaterial>();


    std::vector<MShaderTextureParam>& vTextureParams = *pMaterial->GetTextureParams();

    for (MShaderTextureParam& param : vTextureParams)
    {
        if (param.strName == "FrontDepth")
        {
            param.pTexture = m_pPrevLevelRenderTarget->GetDepthTexture();
        }
    }
}

