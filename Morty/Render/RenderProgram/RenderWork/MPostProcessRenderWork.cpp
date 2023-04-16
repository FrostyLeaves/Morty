#include "MPostProcessRenderWork.h"
#include "MPostProcessRenderWork.h"
#include "MPostProcessRenderWork.h"
#include "MPostProcessRenderWork.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"
#include "Mesh/MMeshManager.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

MORTY_CLASS_IMPLEMENT(MPostProcessRenderWork, ISinglePassRenderWork)

void MPostProcessRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeMaterial();
}

void MPostProcessRenderWork::Release(MEngine* pEngine)
{
	ReleaseMaterial();

	Super::Release(pEngine);
}

void MPostProcessRenderWork::Render(MRenderInfo& info)
{
	MORTY_ASSERT(m_pInputAdapter);

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	auto pInputTexture = m_pInputAdapter->GetTexture();

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	pCommand->AddRenderToTextureBarrier({ pInputTexture.get() });

	pCommand->BeginRenderPass(&m_renderPass);

	Vector2 v2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	m_pMaterial->GetMaterialParamSet()->SetTexture("u_texScreenTexture", pInputTexture);
	if (pCommand->SetUseMaterial(m_pMaterial))
	{
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	pCommand->EndRenderPass();
}

void MPostProcessRenderWork::SetInputTexture(const std::shared_ptr<ITextureInputAdapter>& pAdapter)
{
	m_pInputAdapter = pAdapter;
}

void MPostProcessRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	m_pMaterial = pResourceSystem->CreateResource<MMaterial>("PostProcess Material");

	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	//std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_aces.mps");
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
	m_pMaterial->LoadVertexShader(pVertexShader);
	m_pMaterial->LoadPixelShader(pPixelShader);
	m_pMaterial->SetRasterizerType(MERasterizerType::ECullNone);

	/*
	if (auto pPropertyBlock = m_pMaterial->GetMaterialParamSet())
	{
		pPropertyBlock->SetValue<float>("FilmSlope", 0.88f);
		pPropertyBlock->SetValue<float>("FilmToe", 0.55f);
		pPropertyBlock->SetValue<float>("FilmShoulder", 0.26f);
		pPropertyBlock->SetValue<float>("FilmBlackClip", 0.0f);
		pPropertyBlock->SetValue<float>("FilmWhiteClip", 0.04f);
	}
	*/
}

void MPostProcessRenderWork::ReleaseMaterial()
{
	m_pMaterial = nullptr;
}
