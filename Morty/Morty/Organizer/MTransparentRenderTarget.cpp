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

		//如果当前有Shader使用了ShadowMap，那么进行ShadowMap的更新
		if (SHADER_PARAM_CODE_DDEPTH_FRONT < MShaderBuffer::s_vTextureParams.size())
		{
			MShaderTextureParam* pDepthFrontParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DDEPTH_FRONT];
			if (SHADER_PARAM_CODE_DDEPTH_FRONT == pDepthFrontParam->unCode)
			{
				pDepthFrontParam->pTexture = m_pPrevLevelRenderTarget->GetDepthTexture();
				pRenderer->SetPixelShaderTexture(*pDepthFrontParam);
			}
		}



    }
}

