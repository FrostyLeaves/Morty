#include "MTransparentRenderTarget.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MTexture.h"

#include "MResourceManager.h"
#include "Texture/MTextureResource.h"

#include "MRenderSystem.h"

MTypeIdentifierImplement(MTransparentRenderTarget, MTextureRenderTarget)

MTransparentRenderTarget::MTransparentRenderTarget()
    : MTextureRenderTarget()
    , m_Material(nullptr)
    , m_pSceneDepthTexture(nullptr)
	, m_pBackRenderTarget(nullptr)
    , m_pPrevLevelRenderTarget(nullptr)
    , m_pTransparentMeshes(nullptr)
{
}

MTransparentRenderTarget::~MTransparentRenderTarget()
{
}

void MTransparentRenderTarget::OnCreated()
{
	Super::OnCreated();

	Initialize(MTextureRenderTarget::ERenderBack | MTextureRenderTarget::ERenderDepth, 0, 0, 3);
}

void MTransparentRenderTarget::OnDelete()
{
    m_Material.SetResource(nullptr);
}

void MTransparentRenderTarget::Render(MIRenderer* pRenderer, MIRenderTarget* pRenderTarget, std::vector<MMaterialGroup>* pGroup)
{
	m_pTransparentMeshes = pGroup;
	m_pBackRenderTarget = pRenderTarget;

	pRenderer->Render(this);

	m_pBackRenderTarget = nullptr;
	m_pTransparentMeshes = nullptr;
}

void MTransparentRenderTarget::OnRender(MIRenderer* pRenderer)
{
	if (nullptr == m_pTransparentMeshes)
		return;

	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == pMeshMatrixParam)
		return;

	MShaderParam* pAnimationParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);

	//如果当前有Shader使用了ShadowMap，那么进行ShadowMap的更新
	if (SHADER_PARAM_CODE_DEPTH_FRONT < MShaderBuffer::s_vTextureParams.size())
	{
		MShaderTextureParam* pDepthFrontParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DEPTH_FRONT];
		MShaderTextureParam* pDepthBackParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DEPTH_BACK];
		if (SHADER_PARAM_CODE_DEPTH_FRONT == pDepthFrontParam->unCode)
		{
			if (m_pPrevLevelRenderTarget)
			{
				pDepthFrontParam->pTexture = m_pPrevLevelRenderTarget->GetDepthTexture();
				pRenderer->SetPixelShaderTexture(*pDepthFrontParam);
			}
			else
			{
				MTextureResource* pBlackTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_BLACK);
				pDepthFrontParam->pTexture = pBlackTextureRes->GetTextureTemplate();;
				pRenderer->SetPixelShaderTexture(*pDepthFrontParam);
			}
		}
		if (SHADER_PARAM_CODE_DEPTH_BACK == pDepthBackParam->unCode)
		{
			pDepthBackParam->pTexture = m_pBackRenderTarget->GetDepthTexture();
			pRenderer->SetPixelShaderTexture(*pDepthBackParam);
		}
		
	}

    for (MMaterialGroup& group : *m_pTransparentMeshes)
    {
		MMaterial* pMaterial = group.m_pMaterial;
		//使用材质
		if (!pRenderer->SetUseMaterial(pMaterial, true))
			continue;

		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			MRenderSystem::DrawMeshInstance(pRenderer, pMeshIns, pMeshMatrixParam, pAnimationParam);
		}
    }
}
