#include "MDeferredLightingRenderWork.h"

#include "MVoxelizerRenderWork.h"
#include "MVRSTextureRenderWork.h"
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
#include "Render/MMaterialName.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"


MORTY_CLASS_IMPLEMENT(MDeferredLightingRenderWork, ISinglePassRenderWork)

const MStringId MDeferredLightingRenderWork::DeferredLightingOutput = MStringId("Deferred Lighting Output");


void MDeferredLightingRenderWork::Render(const MRenderInfo& info)
{
	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager)
	{
		MORTY_ASSERT(pMeshManager);
		return;
	}

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	//pCommand->AddRenderToTextureBarrier(m_vInputTexture, METextureBarrierStage::EPixelShaderSample);
	pCommand->AddRenderToTextureBarrier(m_vBarrierTexture, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(&m_renderPass);

	//pCommand->SetShadingRate({ 1, 1 }, { MEShadingRateCombinerOp::Keep, MEShadingRateCombinerOp::Replace });

	const Vector2i n2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


	if (pCommand->SetUseMaterial(m_pLightningMaterial))
	{
		auto pPropertyBlock = GetRenderGraph()->GetFrameProperty()->GetPropertyBlock();
		pCommand->SetShaderPropertyBlock(pPropertyBlock);
		
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	pCommand->EndRenderPass();
}

void MDeferredLightingRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::DEFERRED_LIGHTING);
	m_pLightningMaterial = MMaterial::CreateMaterial(pTemplate);

}

void MDeferredLightingRenderWork::Release()
{
	m_pLightningMaterial = nullptr;

	auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (auto pShadingRateTexture = m_renderPass.GetShadingRateTexture())
	{
		pShadingRateTexture->DestroyBuffer(pRenderSystem->GetDevice());
	}

	Super::Release();
}

void MDeferredLightingRenderWork::BindTarget()
{
	if (std::shared_ptr<MShaderPropertyBlock> pParams = m_pLightningMaterial->GetMaterialPropertyBlock())
	{
		m_vInputTexture = {
			GetInputTexture(0).get(),
			GetInputTexture(1).get(),
			GetInputTexture(2).get(),
			GetInputTexture(3).get(),
		};

		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC, GetInputTexture(0));
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS, GetInputTexture(1));
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC, GetInputTexture(2));
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_DEPTH_MAP, GetInputTexture(3));
	}

	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MStringId> MDeferredLightingRenderWork::GetInputName()
{
	return {
		MGBufferRenderWork::GBufferAlbedoMetallic,
		MGBufferRenderWork::GBufferNormalRoughness,
		MGBufferRenderWork::GBufferPositionAmbientOcc,
		MGBufferRenderWork::GBufferDepthBufferOutput,
		MShadowMapRenderWork::ShadowMapBufferOutput,
#if MORTY_VXGI_ENABLE
		//TODO: this input texture is not be used.
		MVoxelizerRenderWork::VoxelizerBufferOutput
#endif
	};
}

std::vector<MRenderTaskOutputDesc> MDeferredLightingRenderWork::GetOutputName() {
	return {
		{ DeferredLightingOutput, {true, MColor::Black_T }},
	};
}