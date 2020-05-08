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
}

void MTransparentRenderTarget::OnDelete()
{
    m_Material.SetResource(nullptr);
}

void MTransparentRenderTarget::OnRender(MIRenderer* pRenderer)
{
    for (MMaterialGroup& group : *m_pTransparentMeshes)
    {
        MMaterial* pMaterial = group.m_pMaterial;

		std::vector<MShaderTextureParam>& vTextureParams = *pMaterial->GetTextureParams();

		for (MShaderTextureParam& param : vTextureParams)
		{
			if (param.strName == "FrontDepth")
			{
				param.pTexture = m_pPrevLevelRenderTarget->GetDepthTexture();
			}
		}

    }
}

