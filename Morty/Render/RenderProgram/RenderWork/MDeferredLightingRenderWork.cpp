#include "MDeferredLightingRenderWork.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "Resource/MMaterialResource.h"


MORTY_CLASS_IMPLEMENT(MDeferredLightingRenderWork, ISinglePassRenderWork)

void MDeferredLightingRenderWork::Render(MRenderInfo& info)
{
	if (!m_pFramePropertyAdapter)
	{
		MORTY_ASSERT(m_pFramePropertyAdapter);
		return;
	}

	if (!m_pGBufferAdapter)
	{
		MORTY_ASSERT(m_pGBufferAdapter);
		return;
	}

	if (!m_pShadowMapAdapter)
	{
		MORTY_ASSERT(m_pShadowMapAdapter);
		return;
	}

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager)
	{
		MORTY_ASSERT(pMeshManager);
		return;
	}

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	std::vector<MTexture*> vTextures;
	for (auto pTexture : m_pGBufferAdapter->GetBackTextures())
	{
		vTextures.push_back(pTexture.get());
	}
	vTextures.push_back(m_pGBufferAdapter->GetDepthTexture().get());

	if (const auto& pTexture = m_pShadowMapAdapter->GetTexture())
	{
		vTextures.push_back(pTexture.get());
	}

	pCommand->AddRenderToTextureBarrier(vTextures);

	pCommand->BeginRenderPass(&m_renderPass);

	Vector2i n2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


	if (pCommand->SetUseMaterial(m_pLightningMaterial))
	{
		auto pPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
		pCommand->SetShaderPropertyBlock(pPropertyBlock);
		
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	pCommand->EndRenderPass();
}

void MDeferredLightingRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);


	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/Deferred/deferred_lighting.mps");


	m_pLightningMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pLightningMaterial->LoadShader(vs);
	m_pLightningMaterial->LoadShader(ps);

}

void MDeferredLightingRenderWork::Release(MEngine* pEngine)
{
	m_pLightningMaterial = nullptr;

	Super::Release(pEngine);
}

void MDeferredLightingRenderWork::SetGBuffer(const std::shared_ptr<IGBufferAdapter>& pAdapter)
{
	MORTY_ASSERT(m_pGBufferAdapter = pAdapter);

	if (std::shared_ptr<MShaderPropertyBlock> pParams = m_pLightningMaterial->GetMaterialPropertyBlock())
	{
		const auto& vTextures = pAdapter->GetBackTextures();
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC, vTextures[0]);
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS, vTextures[1]);
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC, vTextures[2]);
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_DEPTH_MAP, pAdapter->GetDepthTexture());
	}
}

void MDeferredLightingRenderWork::SetShadowMap(const std::shared_ptr<ITextureInputAdapter>& pAdapter)
{
	m_pShadowMapAdapter = pAdapter;
}

void MDeferredLightingRenderWork::SetFrameProperty(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}
