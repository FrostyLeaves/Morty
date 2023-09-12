#include "MTransparentRenderWork.h"

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

#include "Mesh/MMeshManager.h"
#include "Resource/MTextureResourceUtil.h"

MORTY_CLASS_IMPLEMENT(MTransparentRenderWork, MObject)

void MTransparentRenderWork::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	InitializeTexture();
	InitializeMaterial();
	InitializeFrameShaderParams();


	InitializePeelRenderPass();
	InitializeFillRenderPass();
}

void MTransparentRenderWork::Release(MEngine* pEngine)
{
	ReleaseFillRenderPass();
	ReleasePeelRenderPass();

	ReleaseFrameShaderParams();
	ReleaseMaterial();
	ReleaseTexture();

}

void MTransparentRenderWork::Resize(Vector2 size)
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	if (m_peelRenderPass.GetFrameBufferSize() != size)
	{
		pRenderSystem->ResizeFrameBuffer(m_peelRenderPass, size);
	}

	if (m_fillRenderPass.GetFrameBufferSize() != size)
	{
		pRenderSystem->ResizeFrameBuffer(m_fillRenderPass, size);
	}
}

void MTransparentRenderWork::SetRenderTarget(std::shared_ptr<MTexture> pOutputTexture, std::shared_ptr<MTexture> pDepthTexture)
{
	m_pOutputTexture = pOutputTexture;
	if (m_pOutputTexture)
	{
		Resize(m_pOutputTexture->GetSize());
	}

	m_pDepthTexture = pDepthTexture;
}

void MTransparentRenderWork::Render(MRenderInfo& info)
{
	if (info.m_tTransparentGroupMesh.empty())
	{
		return;
	}

	RenderDepthPeel(info);
	
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

	pCommand->AddRenderToTextureBarrier({ m_pFrontTexture.get(), m_pBackTexture.get() });

	pCommand->BeginRenderPass(&m_fillRenderPass);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));

	pCommand->SetUseMaterial(m_pDrawFillMaterial);

	pCommand->DrawMesh(pMeshManager->GetScreenRect());

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::RenderDepthPeel(MRenderInfo& info)
{
	if (info.m_tTransparentGroupMesh.empty())
		return;

	if (nullptr == info.pViewport)
	{
		MORTY_ASSERT(info.pViewport);
		return;
	}

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

	m_aFramePropertyBlock[0].UpdateShaderSharedParams(info);
	m_aFramePropertyBlock[1].UpdateShaderSharedParams(info);


	pCommand->AddRenderToTextureBarrier({ m_pDepthTexture.get() });

	pCommand->BeginRenderPass(&m_peelRenderPass);


	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	Vector2 v2Size = info.pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	if (m_pDrawPeelMaterial->GetTextureParams()[0]->GetTexture() != m_pDepthTexture)
	{
		m_pDrawPeelMaterial->GetTextureParams()[0]->SetTexture(m_pDepthTexture);
	}
	

	if (pCommand->SetUseMaterial(m_pDrawPeelMaterial))
	{
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	for (uint32_t i = 1; i < m_peelRenderPass.m_vSubpass.size(); ++i)
	{
		pCommand->NextSubpass();

		for (auto& pr : info.m_tTransparentGroupMesh)
		{
			std::shared_ptr<MMaterial> pMaterial = pr.first;
			//ʹ�ò���
			if (!pCommand->SetUseMaterial(pMaterial))
				continue;

			auto pPropertyBlock = m_aFramePropertyBlock[i % 2].GetPropertyBlock();
			pCommand->SetShaderPropertyBlock(pPropertyBlock);
			for (MRenderMeshComponent* pMeshComponent : pr.second)
			{
				const MMeshManager::MMeshData& meshData = pMeshManager->FindMesh(pMeshComponent->GetMesh());

				pCommand->DrawMesh(
					pMeshManager->GetVertexBuffer(),
					pMeshManager->GetIndexBuffer(),
					meshData.vertexMemoryInfo.begin,
					meshData.indexMemoryInfo.begin,
					meshData.indexMemoryInfo.size);
			}
		}
	}

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> pDPVSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");
	std::shared_ptr<MResource> pDPBPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mps");
	std::shared_ptr<MResource> pDPFPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_fill.mps");

	m_pDrawPeelMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pDrawPeelMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	m_pDrawPeelMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawPeelMaterial->LoadPixelShader(pDPFPSResource);


	m_pDrawFillMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pDrawFillMaterial->SetMaterialType(MEMaterialType::ETransparentBlend);
	m_pDrawFillMaterial->LoadVertexShader(pDPVSResource);
	m_pDrawFillMaterial->LoadPixelShader(pDPBPSResource);

	std::vector<std::shared_ptr<MShaderTextureParam>>& params = m_pDrawFillMaterial->GetTextureParams();
	params[0]->SetTexture(m_pFrontTexture);
	params[1]->SetTexture(m_pBackTexture);
}

void MTransparentRenderWork::ReleaseMaterial()
{
	m_pDrawFillMaterial = nullptr;

	m_pDrawPeelMaterial = nullptr;
}

void MTransparentRenderWork::InitializeTexture()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	m_pBlackTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_Black");
	m_pWhiteTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_White");

	static MByte black[4] = { 0, 0 ,0 ,0 };
	static MByte white[4] = { 255, 255 ,255 ,255 };

    m_pBlackTexture->Load(MTextureResourceUtil::LoadFromMemory("Transparent_Black", black, 1, 1, 4));
	m_pWhiteTexture->Load(MTextureResourceUtil::LoadFromMemory("Transparent_White", white, 1, 1, 4));


	Vector2 size(512, 512);


	m_pFrontTexture = MTexture::CreateRenderTarget();
	m_pFrontTexture->SetSize(size);

	m_pBackTexture = MTexture::CreateRenderTarget();
	m_pBackTexture->SetSize(size);
	
	m_pFrontDepthForPassA = MTexture::CreateRenderTargetFloat32();
	m_pFrontDepthForPassA->SetSize(size);
	m_pFrontDepthForPassA->GenerateBuffer(pRenderSystem->GetDevice());

	m_pBackDepthForPassA = MTexture::CreateRenderTargetFloat32();
	m_pBackDepthForPassA->SetSize(size);
	m_pBackDepthForPassA->GenerateBuffer(pRenderSystem->GetDevice());

	m_pFrontDepthForPassB = MTexture::CreateRenderTargetFloat32();
	m_pFrontDepthForPassB->SetSize(size);
	m_pFrontDepthForPassB->GenerateBuffer(pRenderSystem->GetDevice());

	m_pBackDepthForPassB = MTexture::CreateRenderTargetFloat32();
	m_pBackDepthForPassB->SetSize(size);
	m_pBackDepthForPassB->GenerateBuffer(pRenderSystem->GetDevice());

	m_pDefaultOutputTexture = MTexture::CreateRenderTarget();
	m_pDefaultOutputTexture->SetSize(size);
	m_pDefaultOutputTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_pOutputTexture = m_pDefaultOutputTexture;
}

void MTransparentRenderWork::ReleaseTexture()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_pBlackTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());
	m_pWhiteTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());

	m_pBlackTexture = nullptr;
	m_pWhiteTexture = nullptr;

	if (m_pFrontTexture)
	{
		m_pFrontTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pFrontTexture = nullptr;
	}

	if (m_pBackTexture)
	{
		m_pBackTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pBackTexture = nullptr;
	}

	if (m_pFrontDepthForPassA)
	{
		m_pFrontDepthForPassA->DestroyBuffer(pRenderSystem->GetDevice());
		m_pFrontDepthForPassA = nullptr;
	}

	if (m_pBackDepthForPassA)
	{
		m_pBackDepthForPassA->DestroyBuffer(pRenderSystem->GetDevice());
		m_pBackDepthForPassA = nullptr;
	}

	if (m_pFrontDepthForPassB)
	{
		m_pFrontDepthForPassB->DestroyBuffer(pRenderSystem->GetDevice());
		m_pFrontDepthForPassB = nullptr;
	}

	if (m_pBackDepthForPassB)
	{
		m_pBackDepthForPassB->DestroyBuffer(pRenderSystem->GetDevice());
		m_pBackDepthForPassB = nullptr;
	}

	if (m_pDefaultOutputTexture)
	{
		m_pDefaultOutputTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pDefaultOutputTexture = nullptr;
	}
}

void MTransparentRenderWork::InitializePeelRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_peelRenderPass.AddBackTexture(m_pFrontTexture, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pBackTexture, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pFrontDepthForPassA, { true, MColor::White });
	
	m_peelRenderPass.AddBackTexture(m_pBackDepthForPassA, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pFrontDepthForPassB, { true, MColor::White });
	
	m_peelRenderPass.AddBackTexture(m_pBackDepthForPassB, { true, MColor::Black_T });


	static const uint32_t SUB_PASS_NUM = 6;

	m_peelRenderPass.m_vSubpass.push_back(MSubpass());
	MSubpass& subpass = m_peelRenderPass.m_vSubpass.back();

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
		m_peelRenderPass.m_vSubpass.push_back(MSubpass());
		MSubpass& subpass = m_peelRenderPass.m_vSubpass.back();

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

	m_peelRenderPass.SetDepthTestEnable(true);
	m_peelRenderPass.SetDepthWriteEnable(false);
	m_peelRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::ReleasePeelRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_peelRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeFillRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_fillRenderPass.SetDepthTestEnable(false);
	m_fillRenderPass.SetDepthWriteEnable(false);
	m_fillRenderPass.AddBackTexture(m_pOutputTexture, { false, true, MColor::Black_T });

	m_fillRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::ReleaseFillRenderPass()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_fillRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeFrameShaderParams()
{
	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> forwardVS = pResourceSystem->LoadResource("Shader/Forward/model.mvs");
	std::shared_ptr<MResource> forwardPS = pResourceSystem->LoadResource("Shader/Forward/model.mps");
	m_pForwardMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pForwardMaterial->SetCullMode(MECullMode::ECullBack);
	m_pForwardMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	m_pForwardMaterial->LoadVertexShader(forwardVS);
	m_pForwardMaterial->LoadPixelShader(forwardPS);

	m_aFramePropertyBlock[0].BindMaterial(m_pForwardMaterial);
	m_aFramePropertyBlock[0].m_pTransparentFrontTextureParam->SetTexture(m_pFrontDepthForPassB);
	m_aFramePropertyBlock[0].m_pTransparentBackTextureParam->SetTexture(m_pBackDepthForPassB);

	m_aFramePropertyBlock[1].BindMaterial(m_pForwardMaterial);
	m_aFramePropertyBlock[1].m_pTransparentFrontTextureParam->SetTexture(m_pFrontDepthForPassA);
	m_aFramePropertyBlock[1].m_pTransparentBackTextureParam->SetTexture(m_pBackDepthForPassA);
}

void MTransparentRenderWork::ReleaseFrameShaderParams()
{
	m_aFramePropertyBlock[0].Release(GetEngine());
	m_aFramePropertyBlock[1].Release(GetEngine());

	m_pForwardMaterial = nullptr;
}
