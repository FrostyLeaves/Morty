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
#include "Component/MRenderableMeshComponent.h"
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

	Vector2 v2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	if (pCommand->SetUseMaterial(m_pLightningMaterial))
	{
		auto vPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
		for (const auto& property : vPropertyBlock)
		{
			pCommand->SetShaderParamSet(property);
		}
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	pCommand->EndRenderPass();
}

void MDeferredLightingRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);


	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/model_deferred.mps");


	m_pLightningMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pLightningMaterial->LoadVertexShader(vs);
	m_pLightningMaterial->LoadPixelShader(ps);

}

void MDeferredLightingRenderWork::Release(MEngine* pEngine)
{
	m_pLightningMaterial = nullptr;

	Super::Release(pEngine);
}

void MDeferredLightingRenderWork::SetGBuffer(const std::shared_ptr<IGBufferAdapter>& pAdapter)
{
	MORTY_ASSERT(m_pGBufferAdapter = pAdapter);

	if (std::shared_ptr<MShaderPropertyBlock>& pParams = m_pLightningMaterial->GetMaterialParamSet())
	{
		const auto& vTextures = pAdapter->GetBackTextures();
		pParams->SetTexture("u_mat_f3Albedo_fMetallic", vTextures[0]);
		pParams->SetTexture("u_mat_f3Normal_fRoughness", vTextures[1]);
		pParams->SetTexture("u_mat_f3Position_fAmbientOcc", vTextures[2]);
		pParams->SetTexture("u_mat_DepthMap", pAdapter->GetDepthTexture());
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

class MLightingOutput : public ITextureInputAdapter
{
public:

	virtual std::shared_ptr<MTexture> GetTexture() { return pTexture; }

	std::shared_ptr<MTexture> pTexture = nullptr;
};

std::shared_ptr<ITextureInputAdapter> MDeferredLightingRenderWork::CreateOutput() const
{
	auto pOutput = std::make_shared< MLightingOutput>();
	pOutput->pTexture = m_renderPass.m_vBackTextures[0].pTexture;

	return pOutput;
}
