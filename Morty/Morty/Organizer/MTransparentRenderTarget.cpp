#include "MTransparentRenderTarget.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MTexture.h"
#include "MIDevice.h"

#include "MResourceManager.h"
#include "Texture/MTextureResource.h"

#include "MRenderSystem.h"

MTypeIdentifierImplement(MTransparentRenderTarget, MTextureRenderTarget)

MTransparentRenderTarget::MTransparentRenderTarget()
    : MTextureRenderTarget()
    , m_Material(nullptr)
	, m_pBackRenderTarget(nullptr)
    , m_pFrontDepthTexture(nullptr)
	, m_pBackDepthTexture(nullptr)
    , m_pTransparentMeshes(nullptr)
	, m_pBlackTexture(nullptr)
	, m_pWhiteTexture(nullptr)
{
}

MTransparentRenderTarget::~MTransparentRenderTarget()
{
}

void MTransparentRenderTarget::OnCreated()
{
	Super::OnCreated();

	std::vector<MERenderTextureType> vTextureTypes(6);
	vTextureTypes[0] = MERenderTextureType::ERGBA8;
	vTextureTypes[1] = MERenderTextureType::ERGBA8;
	vTextureTypes[2] = MERenderTextureType::ER24X8;
	vTextureTypes[3] = MERenderTextureType::ER24X8;
	vTextureTypes[4] = MERenderTextureType::ER24X8;
	vTextureTypes[5] = MERenderTextureType::ER24X8;

	Initialize(MTextureRenderTarget::ERenderBack | MTextureRenderTarget::ERenderDepth, 0, 0, vTextureTypes);

	m_vBackgroundColor[0] = MColor::Black_T;
	m_vBackgroundColor[1] = MColor::Black_T;
	m_vBackgroundColor[2] = MColor::White;
	m_vBackgroundColor[3] = MColor::Black_T;
	m_vBackgroundColor[4] = MColor::White;
	m_vBackgroundColor[5] = MColor::Black_T;

	MTextureResource* pBlackTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_BLACK);
	MTextureResource* pWhiteTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_WHITE);

	m_pBlackTexture = pBlackTextureRes->GetTextureTemplate();
	m_pWhiteTexture = pWhiteTextureRes->GetTextureTemplate();
}

void MTransparentRenderTarget::OnDelete()
{
    m_Material.SetResource(nullptr);
}

void MTransparentRenderTarget::Render(MIRenderer* pRenderer, MIRenderTarget* pRenderTarget, std::vector<MMaterialGroup>* pGroup)
{
	m_unTargetViewNum = 4;
	m_pTransparentMeshes = pGroup;
	m_pBackRenderTarget = pRenderTarget;

	for (unsigned int i = 0; i < 3; ++i)
	{
		m_vNeedCleanBeforeRender[0] = false;
		m_vNeedCleanBeforeRender[1] = false;

		if (i % 2 == 0)
		{
			m_vpRenderTargetView[2] = m_vBackTexture[2].GetRenderBuffer()->m_pRenderTargetView;
			m_vpRenderTargetView[3] = m_vBackTexture[3].GetRenderBuffer()->m_pRenderTargetView;

			if (i == 0)
			{
				m_pFrontDepthTexture = m_pBlackTexture;
				m_pBackDepthTexture = m_pWhiteTexture;

				m_vNeedCleanBeforeRender[0] = true;
				m_vNeedCleanBeforeRender[1] = true;
			}
			else
			{
				m_pFrontDepthTexture = &m_vBackTexture[4];
				m_pBackDepthTexture = &m_vBackTexture[5];
			}
		}
		else
		{
			m_vpRenderTargetView[2] = m_vBackTexture[4].GetRenderBuffer()->m_pRenderTargetView;
			m_vpRenderTargetView[3] = m_vBackTexture[5].GetRenderBuffer()->m_pRenderTargetView;

			m_pFrontDepthTexture = &m_vBackTexture[2];
			m_pBackDepthTexture = &m_vBackTexture[3];
		}

		pRenderer->Render(this, pRenderTarget->GetDepthTexture());
	}
	
	m_pBackRenderTarget = nullptr;
	m_pTransparentMeshes = nullptr;
	m_unTargetViewNum = 6;
}

void MTransparentRenderTarget::OnRender(MIRenderer* pRenderer)
{

	if (nullptr == m_pTransparentMeshes)
		return;

	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == pMeshMatrixParam)
		return;

	MShaderParam* pAnimationParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);


	//更新上一层的DepthMap
	if (SHADER_PARAM_CODE_DEPTH_FRONT < MShaderBuffer::s_vTextureParams.size())
	{
		MShaderTextureParam* pDepthFrontParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DEPTH_FRONT];
		MShaderTextureParam* pDepthBackParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DEPTH_BACK];

		if (SHADER_PARAM_CODE_DEPTH_FRONT == pDepthFrontParam->unCode)
		{
			if (m_pFrontDepthTexture)
			{
				pDepthFrontParam->pTexture = m_pFrontDepthTexture;
				pRenderer->SetPixelShaderTexture(*pDepthFrontParam);
			}
			if (m_pFrontDepthTexture)
			{
				pDepthBackParam->pTexture = m_pBackDepthTexture;
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


		pDepthFrontParam->pTexture = nullptr;
		pDepthBackParam->pTexture = nullptr;
		pRenderer->SetPixelShaderTexture(*pDepthFrontParam);
		pRenderer->SetPixelShaderTexture(*pDepthBackParam);
	}

  


}
