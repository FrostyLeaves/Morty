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
#include "PostProcess/MPostProcessGraphWalker.h"
#include "PostProcess/MPostProcessNode.h"
#include "Mesh/MMeshManager.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

MORTY_CLASS_IMPLEMENT(MPostProcessRenderWork, IRenderWork)

void MPostProcessRenderWork::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	InitializeMaterial();
}

void MPostProcessRenderWork::Release(MEngine* pEngine)
{
	MORTY_UNUSED(pEngine);

	ReleaseMaterial();
}

void MPostProcessRenderWork::Render(MRenderInfo& info)
{
	MORTY_ASSERT(m_pInputAdapter);

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	MIMesh* pScreenMesh = GetEngine()->FindGlobalObject<MMeshManager>()->GetScreenRect();
	auto pInputTexture = m_pInputAdapter->GetTexture();

	pCommand->AddRenderToTextureBarrier({ pInputTexture.get() }, METextureBarrierStage::EPixelShaderSample);

	for (auto pStartNode : m_postProcessGraph.GetStartNodes())
	{
		auto pProcessNode = pStartNode->DynamicCast<MPostProcessNode>();

		pProcessNode->GetMaterial()->GetMaterialPropertyBlock()->SetTexture(MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE, pInputTexture);
	}

	MPostProcessGraphWalker walker(pCommand, pScreenMesh);
	m_postProcessGraph.Run(&walker);
}

void MPostProcessRenderWork::Resize(Vector2i size)
{
	m_pRenderTargetBinding->Resize(size);
}

void MPostProcessRenderWork::SetInputTexture(const std::shared_ptr<IGetTextureAdapter>& pAdapter)
{
	m_pInputAdapter = pAdapter;
}

void MPostProcessRenderWork::SetRenderTarget(const MRenderTarget& backTexture)
{
	MORTY_ASSERT(m_postProcessGraph.GetFinalNodes().size() == 1);
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	for (auto pNode : m_postProcessGraph.GetFinalNodes())
	{
		auto pPostProcessNode = pNode->DynamicCast<MPostProcessNode>();
		MRenderPass* pRenderPass = pPostProcessNode->GetRenderPass();

		pRenderPass->m_vBackTextures.clear();
		pRenderPass->AddBackTexture(backTexture.pTexture, backTexture.desc);
		pRenderPass->DestroyBuffer(pRenderSystem->GetDevice());
		pRenderPass->GenerateBuffer(pRenderSystem->GetDevice());
	}
}

void MPostProcessRenderWork::InitializeMaterial()
{
	m_pRenderTargetBinding = std::make_unique<MPostProcessRenderTargetBinding>(GetEngine());

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pMaterial = pResourceSystem->CreateResource<MMaterial>("PostProcess Material");

	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
	pMaterial->LoadShader(pVertexShader);
	pMaterial->LoadShader(pPixelShader);
	pMaterial->SetCullMode(MECullMode::ECullNone);

	auto pBasicProcess = m_postProcessGraph.AddNode<MPostProcessNode>("basic_process");
	pBasicProcess->SetMaterial(pMaterial);


	//generate and bind render target.
	m_postProcessGraph.Run(m_pRenderTargetBinding.get());

}

void MPostProcessRenderWork::ReleaseMaterial()
{
	//TODO: destroy material.

	m_pRenderTargetBinding->Release();
	m_pRenderTargetBinding = nullptr;
}
