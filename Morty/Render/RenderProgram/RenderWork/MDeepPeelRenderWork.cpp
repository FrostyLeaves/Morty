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

MORTY_CLASS_IMPLEMENT(MDeepPeelRenderWork, ISinglePassRenderWork)

const MStringId MDeepPeelRenderWork::FrontTextureOutput = MStringId("Deep Peel Front Buffer Output");
const MStringId MDeepPeelRenderWork::BackTextureOutput = MStringId("Deep Peel Back Buffer Output");
const MStringId MDeepPeelRenderWork::DepthOutput[4] = {
	MStringId("Deep Peel Front Depth Output 0"),
	MStringId("Deep Peel Back Depth Output 0"),
	MStringId("Deep Peel Front Buffer Output 1"),
	MStringId("Deep Peel Back Depth Output 1"),
};


void MDeepPeelRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeTexture();
	InitializeMaterial();
	InitializeFrameShaderParams();


	InitializeRenderPass();
}

void MDeepPeelRenderWork::Release()
{
	
	ReleaseFrameShaderParams();
	ReleaseMaterial();
	ReleaseTexture();

	Super::Release();
}

void MDeepPeelRenderWork::Render(const MRenderInfo& info)
{
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		GetRenderGraph()->GetFrameProperty(),
		});
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDepthPeel));
	indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

	Render(info, { &indirectMesh });
}

void MDeepPeelRenderWork::Render(const MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable)
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
	pCommand->SetScissor(MScissorInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));



	if (pCommand->SetUseMaterial(m_pDrawPeelMaterial))
	{
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	for (size_t nSubpassIdx = 1; nSubpassIdx < m_renderPass.m_vSubpass.size(); ++nSubpassIdx)
	{
		pCommand->NextSubPass();

		pCommand->PushShaderPropertyBlock(m_aFramePropertyBlock[nSubpassIdx % 2]);

		for (MCullingResultRenderable* pRenderable : vRenderable)
		{
			pRenderable->Render(pCommand);
		}

		pCommand->PopShaderPropertyBlock();
	}

	pCommand->EndRenderPass();
}

void MDeepPeelRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> pDPVSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");
	std::shared_ptr<MResource> pDPFPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_fill.mps");

	const auto pPeelTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
	pPeelTemplate->SetMaterialType(MEMaterialType::EDepthPeel);
	pPeelTemplate->AddDefine(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	pPeelTemplate->LoadShader(pDPVSResource);
	pPeelTemplate->LoadShader(pDPFPSResource);
	m_pDrawPeelMaterial = MMaterial::CreateMaterial(pPeelTemplate);
}

void MDeepPeelRenderWork::ReleaseMaterial()
{
	m_pDrawPeelMaterial = nullptr;
}

void MDeepPeelRenderWork::InitializeTexture()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	static MByte black[4] = { 0, 0 ,0 ,0 };
	static MByte white[4] = { 255, 255 ,255 ,255 };

	m_pBlackTexture = pResourceSystem->FindResource<MTextureResource>("Transparent_Black");
	if (m_pBlackTexture == nullptr)
	{
		m_pBlackTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_Black");
		m_pBlackTexture->Load(MTextureResourceUtil::LoadFromMemory("Transparent_Black", black, 1, 1, 4));
	}

	m_pWhiteTexture = pResourceSystem->FindResource<MTextureResource>("Transparent_White");
	if (m_pWhiteTexture == nullptr)
	{
		m_pWhiteTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_White");
		m_pWhiteTexture->Load(MTextureResourceUtil::LoadFromMemory("Transparent_White", white, 1, 1, 4));
	}

}

void MDeepPeelRenderWork::ReleaseTexture()
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pBlackTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());
	m_pWhiteTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());

	m_pBlackTexture = nullptr;
	m_pWhiteTexture = nullptr;

}

void MDeepPeelRenderWork::InitializeRenderPass()
{
#if MORTY_DEBUG
	m_renderPass.m_strDebugName = "Transparent Peel";
#endif

	static constexpr uint32_t SUB_PASS_NUM = 6;

	m_renderPass.m_vSubpass.push_back(MSubpass());
	MSubpass& subpass = m_renderPass.m_vSubpass.back();

	/*
	* 0 output front
	* 1 output back
	* 2 input/output front
	* 3 input/output back
	* 4 input/output front depth
	* 5 input/output back depth
	*/

	subpass.m_vInputIndex = {};
	subpass.m_vOutputIndex = { 0, 1, 2, 3 };

	for (uint32_t i = 0; i < SUB_PASS_NUM; ++i)
	{
		m_renderPass.m_vSubpass.push_back(MSubpass());
		MSubpass& subpass = m_renderPass.m_vSubpass.back();

		if (i % 2)
		{
			subpass.m_vInputIndex = { 4, 5 };
			subpass.m_vOutputIndex = { 0, 1, 2, 3 };
		}
		else
		{
			subpass.m_vInputIndex = { 2, 3 };
			subpass.m_vOutputIndex = { 0, 1, 4, 5 };
		}
	}

	m_renderPass.SetDepthTestEnable(true);
	m_renderPass.SetDepthWriteEnable(false);
}

void MDeepPeelRenderWork::InitializeFrameShaderParams()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> forwardVS = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
	std::shared_ptr<MResource> forwardPS = pResourceSystem->LoadResource("Shader/Forward/basic_lighting.mps");
	auto pTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
	pTemplate->SetCullMode(MECullMode::ECullBack);
	pTemplate->SetMaterialType(MEMaterialType::EDepthPeel);
	pTemplate->AddDefine(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	pTemplate->LoadShader(forwardVS);
	pTemplate->LoadShader(forwardPS);

	m_pForwardMaterial = MMaterial::CreateMaterial(pTemplate);

	m_aFramePropertyBlock[0] = m_pForwardMaterial->GetShaderProgram()->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]->Clone();
	m_aFramePropertyBlock[1] = m_pForwardMaterial->GetShaderProgram()->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]->Clone();
}

void MDeepPeelRenderWork::ReleaseFrameShaderParams()
{
	const auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_aFramePropertyBlock[0]->DestroyBuffer(pRenderSystem->GetDevice());
	m_aFramePropertyBlock[1]->DestroyBuffer(pRenderSystem->GetDevice());

	m_pForwardMaterial = nullptr;
}

void MDeepPeelRenderWork::BindTarget()
{
	const auto pDepthTexture = GetInputTexture(1);
	m_pDrawPeelMaterial->GetMaterialPropertyBlock()->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_BACK_TEXTURE, pDepthTexture);

	m_aFramePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, GetOutputTexture(4));
	m_aFramePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, GetOutputTexture(5));
	m_aFramePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, GetOutputTexture(2));
	m_aFramePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, GetOutputTexture(3));

	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTarget());
}

std::vector<MStringId> MDeepPeelRenderWork::GetInputName()
{
	return {
		   MForwardRenderWork::BackBufferOutput,
		   MForwardRenderWork::DepthBufferOutput,
	};
}

std::vector<MRenderTaskOutputDesc> MDeepPeelRenderWork::GetOutputName()
{
	return {
		{ FrontTextureOutput, {true, MColor::Black_T } },
		{ BackTextureOutput, {true, MColor::Black_T } },
		{ DepthOutput[0], {true, MColor::White}},
		{ DepthOutput[1], {true, MColor::Black_T}},
		{ DepthOutput[2], {true, MColor::White}},
		{ DepthOutput[3], {true, MColor::Black_T}},
	};
}
