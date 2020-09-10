#include "MTransparentRenderTarget.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MTexture.h"
#include "MIDevice.h"
#include "MViewport.h"

#include "MResourceManager.h"
#include "Shader/MShaderBuffer.h"
#include "Texture/MTextureResource.h"

M_OBJECT_IMPLEMENT(MTransparentRenderTarget, MTextureRenderTarget)

MTransparentRenderTarget::MTransparentRenderTarget()
    : MTextureRenderTarget()
	, m_Material(nullptr)
    , m_pFrontDepthTexture(nullptr)
	, m_pBackDepthTexture(nullptr)
    , m_pTransparentMeshes(nullptr)
	, m_pViewport(nullptr)
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

// 	std::vector<MERenderTextureType> vTextureTypes(6);
// 	vTextureTypes[0] = MERenderTextureType::ERGBA8;
// 	vTextureTypes[1] = MERenderTextureType::ERGBA8;
// 	vTextureTypes[2] = MERenderTextureType::ER32;
// 	vTextureTypes[3] = MERenderTextureType::ER32;
// 	vTextureTypes[4] = MERenderTextureType::ER32;
// 	vTextureTypes[5] = MERenderTextureType::ER32;
// 
// 	Initialize(MTextureRenderTarget::ERenderBack | MTextureRenderTarget::ERenderDepth, 0, 0, vTextureTypes);



	MTextureResource* pBlackTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_BLACK);
	MTextureResource* pWhiteTextureRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_WHITE);

	m_pBlackTexture = pBlackTextureRes->GetTextureTemplate();
	m_pWhiteTexture = pWhiteTextureRes->GetTextureTemplate();
}

void MTransparentRenderTarget::OnDelete()
{
    m_Material.SetResource(nullptr);
}

void MTransparentRenderTarget::Render(MIRenderer* pRenderer, MViewport* pViewport, MIRenderTarget* pRenderTarget, std::vector<MMaterialGroup>* pGroup)
{
	m_pTransparentMeshes = pGroup;
	m_pViewport = pViewport;

	//SetDepthTexture(pRenderTarget->GetCurrDepthTexture(), false);

	//pRenderer->Render(this);

	m_pTransparentMeshes = nullptr;
	m_pViewport = nullptr;

}

void MTransparentRenderTarget::ResetPrevLayerTexture()
{
	m_pFrontDepthTexture = m_pBlackTexture;
	m_pBackDepthTexture = m_pWhiteTexture;
}

void MTransparentRenderTarget::OnRender(MIRenderer* pRenderer)
{
// 
// 	if (nullptr == m_pTransparentMeshes)
// 		return;
// 
// 	if (nullptr == m_pViewport)
// 		return;
// 
// 
// 	MIRenderProgram* pRenderProgram = m_pViewport->GetRenderProgram();
// 	if (nullptr == pRenderProgram)
// 		return;
// 
// 	MShaderConstantParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
// 	if (nullptr == pMeshMatrixParam)
// 		return;
// 
// 	MShaderConstantParam* pAnimationParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);

// 
// 	//更新上一层的DepthMap
// 	if (SHADER_PARAM_CODE_DEPTH_FRONT < MShaderBuffer::s_vTextureParams.size())
// 	{
// 		MShaderTextureParam* pDepthFrontParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DEPTH_FRONT];
// 		MShaderTextureParam* pDepthBackParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_DEPTH_BACK];
// 
// 		if (SHADER_PARAM_CODE_DEPTH_FRONT == pDepthFrontParam->unCode)
// 		{
// 			if (m_pFrontDepthTexture)
// 			{
// 				pDepthFrontParam->pTexture = m_pFrontDepthTexture;
// 				pRenderer->SetShaderTexture(*pDepthFrontParam);
// 			}
// 			if (m_pFrontDepthTexture)
// 			{
// 				pDepthBackParam->pTexture = m_pBackDepthTexture;
// 				pRenderer->SetShaderTexture(*pDepthBackParam);
// 			}
// 		}
// 
// 		for (MMaterialGroup& group : *m_pTransparentMeshes)
// 		{
// 			MMaterial* pMaterial = group.m_pMaterial;
// 			//使用材质
// 			if (!pRenderer->SetUseMaterial(pMaterial, true))
// 				continue;
// 
// 			for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
// 			{
// 				pRenderProgram->DrawMeshInstance(pRenderer, pMeshIns, pMeshMatrixParam, pAnimationParam);
// 			}
// 		}
// 
// 
// 		pDepthFrontParam->pTexture = nullptr;
// 		pDepthBackParam->pTexture = nullptr;
// 		pRenderer->SetShaderTexture(*pDepthFrontParam);
// 		pRenderer->SetShaderTexture(*pDepthBackParam);
// 	}

  


}
