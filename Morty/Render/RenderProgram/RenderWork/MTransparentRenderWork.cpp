#include "MTransparentRenderWork.h"

#include "MDeepPeelRenderWork.h"
#include "MForwardRenderWork.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Model/MSkeletonInstance.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"

#include "Component/MRenderMeshComponent.h"
#include "Culling/MInstanceCulling.h"

#include "Mesh/MMeshManager.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Resource/MTextureResourceUtil.h"
#include "Utility/MGlobal.h"

MORTY_CLASS_IMPLEMENT(MTransparentRenderWork, ISinglePassRenderWork)

const MStringId MTransparentRenderWork::BackBufferOutput = MStringId("Transparent Back Buffer Output");


void MTransparentRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeMaterial();
	InitializeFillRenderPass();
}

void MTransparentRenderWork::Release()
{
	ReleaseMaterial();

	Super::Release();
}

void MTransparentRenderWork::Render(const MRenderInfo& info)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
	{
		MORTY_ASSERT(pCommand);
		return;
	}

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager)
	{
		MORTY_ASSERT(pMeshManager);
		return;
	}

	pCommand->AddRenderToTextureBarrier(m_vBarrierTexture, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(&m_renderPass);

	const Vector2i f2LeftTop = info.f2ViewportLeftTop;
	const Vector2i f2Size = info.f2ViewportSize;
	pCommand->SetViewport(MViewportInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, f2Size.x, f2Size.y));

	pCommand->SetUseMaterial(m_pDrawFillMaterial);

	pCommand->DrawMesh(pMeshManager->GetScreenRect());

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> pDPVSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");
	std::shared_ptr<MResource> pDPBPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mps");


	const auto pFillTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
	pFillTemplate->SetMaterialType(MEMaterialType::ETransparentBlend);
	pFillTemplate->LoadShader(pDPVSResource);
	pFillTemplate->LoadShader(pDPBPSResource);
	m_pDrawFillMaterial = MMaterial::CreateMaterial(pFillTemplate);

}

void MTransparentRenderWork::ReleaseMaterial()
{
	m_pDrawFillMaterial = nullptr;
}

void MTransparentRenderWork::InitializeFillRenderPass()
{
#if MORTY_DEBUG
	m_renderPass.m_strDebugName = "Transparent Fill";
#endif

	m_renderPass.SetDepthTestEnable(false);
	m_renderPass.SetDepthWriteEnable(false);
}
                                  
void MTransparentRenderWork::BindTarget()
{
	std::vector<std::shared_ptr<MShaderTextureParam>>& params = m_pDrawFillMaterial->GetMaterialPropertyBlock()->m_vTextures;
	params[0]->SetTexture(GetInputTexture(0));
	params[1]->SetTexture(GetInputTexture(1));

	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTarget());
}

std::vector<MStringId> MTransparentRenderWork::GetInputName()
{
	return {
		   MDeepPeelRenderWork::FrontTextureOutput,
		   MDeepPeelRenderWork::BackTextureOutput,
	};
}

std::vector<MRenderTaskOutputDesc> MTransparentRenderWork::GetOutputName()
{
	return {
		{ BackBufferOutput, {false, MColor::Black_T } }
	};
}
