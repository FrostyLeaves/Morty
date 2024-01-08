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

	MPostProcessGraphWalker walker(pCommand, pScreenMesh, m_pRenderTargetBinding->GetFrameProperty());
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
	for (auto pNode : m_postProcessGraph.GetFinalNodes())
	{
		if (pNode->GetNodeName() == MRenderGlobal::POSTPROCESS_FINAL_NODE)
		{
			if (auto pPostProcessNode = pNode->DynamicCast<MPostProcessNode>())
			{
				pPostProcessNode->SetRenderTarget(backTexture);
			}
		}
	}

	//TODO: once only.
	//generate and bind render target.
	m_postProcessGraph.Run(m_pRenderTargetBinding.get());
}

std::shared_ptr<IGetTextureAdapter> MPostProcessRenderWork::GetOutput(const MStringId& strNodeName) const
{
	for (auto pNode : m_postProcessGraph.GetFinalNodes())
	{
		if (pNode->GetNodeName() == strNodeName)
		{
			auto pPostProcessNode = pNode->DynamicCast<MPostProcessNode>();
			if (MRenderPass* pRenderPass = pPostProcessNode->GetRenderPass())
			{
				if (!pRenderPass->GetBackTextures().empty())
				{
					return std::make_shared<MGetTextureAdapter>(pRenderPass->GetBackTexture(0));
				}
			}
		}
	}

	return nullptr;
}

void MPostProcessRenderWork::InitializeMaterial()
{
	m_pRenderTargetBinding = std::make_unique<MPostProcessRenderTargetBinding>(GetEngine());

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Basic");
	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
	pMaterial->LoadShader(pVertexShader);
	pMaterial->LoadShader(pPixelShader);
	pMaterial->SetCullMode(MECullMode::ECullNone);

	auto pBasicProcess = m_postProcessGraph.AddNode<MPostProcessNode>(MRenderGlobal::POSTPROCESS_FINAL_NODE);
	pBasicProcess->SetMaterial(MMaterial::CreateMaterial(pMaterial));

	auto pEdgeMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Edge Detection");
	std::shared_ptr<MResource> pEdgePixelShader = pResourceSystem->LoadResource("Shader/PostProcess/sobel_edge_detection.mps");
	pEdgeMaterial->LoadShader(pVertexShader);
	pEdgeMaterial->LoadShader(pEdgePixelShader);
	pEdgeMaterial->SetCullMode(MECullMode::ECullNone);

	auto pEdgeDetection = m_postProcessGraph.AddNode<MPostProcessNode>(MRenderGlobal::POSTPROCESS_EDGE_DETECTION);
	pEdgeDetection->SetMaterial(MMaterial::CreateMaterial(pEdgeMaterial));


	m_postProcessGraph.Compile();
}

void MPostProcessRenderWork::ReleaseMaterial()
{
	//TODO: destroy material.

	m_pRenderTargetBinding->Release();
	m_pRenderTargetBinding = nullptr;
}
