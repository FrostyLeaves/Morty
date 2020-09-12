#include "MShadowTextureRenderTarget.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MIRenderer.h"
#include "MMaterial.h"
#include "MScene.h"
#include "MViewport.h"

#include "Model/MModelInstance.h"
#include "Model/MSkinnedMeshInstance.h"
#include "MSkeleton.h"

#include "Shader/MShaderBuffer.h"


M_OBJECT_IMPLEMENT(MShadowTextureRenderTarget, MTextureRenderTarget)

MShadowTextureRenderTarget::MShadowTextureRenderTarget()
	: MTextureRenderTarget()
	, m_m4LightInvProj(Matrix4::IdentityMatrix)
	, m_pShadowRenderGroup(nullptr)
	, m_pStaticMaterial(nullptr)
	, m_pAnimMaterial(nullptr)
	, m_pMeshParam(nullptr)
	, m_pWorldParam(nullptr)
	, m_pAnimBonesParam(nullptr)
{

}

MShadowTextureRenderTarget::~MShadowTextureRenderTarget()
{

}

void MShadowTextureRenderTarget::Render(MIRenderer* pRenderer, const Matrix4& m4InvProj, std::vector<MShadowRenderGroup>* pGroup)
{
	m_m4LightInvProj = m4InvProj;

	m_pShadowRenderGroup = pGroup;

	pRenderer->BeginRenderPass(this);

	OnRender(pRenderer);

	pRenderer->EndRenderPass(this);

	m_pShadowRenderGroup = nullptr;
}

void MShadowTextureRenderTarget::OnCreated()
{

	Super::OnCreated();

	MMaterialResource* pShadowMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW);
	m_pStaticMaterial = pShadowMaterialRes;

	MMaterialResource* pShadowWithAnimMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_ANIM);
	m_pAnimMaterial = pShadowWithAnimMaterialRes;
	
//	std::vector<METextureLayout> vEmpty;
//	Initialize(MTextureRenderTarget::ERenderDepth, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE, vEmpty);

	//InitRenderPass();
}

void MShadowTextureRenderTarget::OnDelete()
{
	Super::OnDelete();
}

void MShadowTextureRenderTarget::OnRender(MIRenderer* pRenderer)
{
	
}

void MShadowTextureRenderTarget::InitRenderPass()
{
	m_RenderPass.m_vSubpass.push_back(MSubpass());

	m_RenderPass.m_DepthDesc.bClearWhenRender = true;
}
