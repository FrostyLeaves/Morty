#include "MForwardHDRWork.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MCombineWork.h"
#include "MGaussianBlurWork.h"

M_OBJECT_IMPLEMENT(MForwardHDRWork, MIPostProcessWork)

MForwardHDRWork::MForwardHDRWork()
    : MIPostProcessWork()
	, m_pRenderProgram(nullptr)
	, m_pTempRenderTarget(nullptr)
	, m_pTempRenderPass(nullptr)
	, m_pScreenDrawMesh(nullptr)
	, m_pHDRMaterial(nullptr)
	, m_aBackTexture()
	, m_aHighLightTexture()
	, m_pGaussianBlurWork(nullptr)
	, m_pCombineWork(nullptr)
{
}

MForwardHDRWork::~MForwardHDRWork()
{
}

void MForwardHDRWork::Render(MPostProcessRenderInfo& info)
{
	info.pRenderer->BeginRenderPass(m_pTempRenderPass, m_pTempRenderTarget);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pHDRMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = info.pPrevLevelOutput;
		pMaterialParamSet->m_vTextures[0]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(m_pHDRMaterial))
	{
		info.pRenderer->DrawMesh(m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass();


	RenderBloom(info);

	RenderCombine(info);
}

MTextureRenderTarget* MForwardHDRWork::GetRenderTarget()
{
	if (m_pCombineWork)
		return m_pCombineWork->GetRenderTarget();
	
	return m_pTempRenderTarget;
}

void MForwardHDRWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	Super::Initialize(pRenderProgram);

	InitializeMesh();
	InitializeMaterial();
	InitializeRenderTargets();
	InitializeRenderPass();

	if (!m_pGaussianBlurWork)
	{
		m_pGaussianBlurWork = GetEngine()->GetObjectManager()->CreateObject<MGaussianBlurWork>();
		m_pGaussianBlurWork->Initialize(pRenderProgram);
	}

	if (!m_pCombineWork)
	{
		m_pCombineWork = GetEngine()->GetObjectManager()->CreateObject<MCombineWork>();
		m_pCombineWork->Initialize(pRenderProgram);
	}
}

void MForwardHDRWork::Release()
{
	if (m_pGaussianBlurWork)
	{
		m_pGaussianBlurWork->DeleteLater();
		m_pGaussianBlurWork = nullptr;
	}

	if (m_pCombineWork)
	{
		m_pCombineWork->DeleteLater();
		m_pCombineWork = nullptr;
	}

	ReleaseRenderPass();
	ReleaseRenderTargets();
	ReleaseMaterial();
	ReleaseMesh();

	Super::Release();
}

void MForwardHDRWork::InitializeMaterial()
{
	m_pHDRMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();

	MResource* pVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/post_process_basic.mvs");
	MResource* pPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/high_dynamic_range.mps");

	m_pHDRMaterial->LoadVertexShader(pVSResource);
	m_pHDRMaterial->LoadPixelShader(pPSResource);

	m_pHDRMaterial->AddRef();
}

void MForwardHDRWork::ReleaseMaterial()
{
	m_pHDRMaterial->SubRef();
	m_pHDRMaterial = nullptr;
}

void MForwardHDRWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MForwardHDRWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MForwardHDRWork::InitializeRenderTargets()
{
	m_pTempRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_aBackTexture[i] = new MRenderBackTexture();
		m_aHighLightTexture[i] = new MRenderBackTexture();
	}

	m_pTempRenderTarget->SetBackTexture(m_aBackTexture, 0);
	m_pTempRenderTarget->SetBackTexture(m_aHighLightTexture, 1);

	m_pTempRenderTarget->Resize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
}

void MForwardHDRWork::ReleaseRenderTargets()
{
	if (m_pTempRenderTarget)
	{
		m_pEngine->GetDevice()->DestroyRenderTarget(m_pTempRenderTarget);
		m_pTempRenderTarget->DeleteLater();
		m_pTempRenderTarget = nullptr;
	}

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (m_aBackTexture[i])
		{
			m_aBackTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			delete m_aBackTexture[i];
			m_aBackTexture[i] = nullptr;
		}

		if (m_aHighLightTexture[i])
		{
			m_aHighLightTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			delete m_aHighLightTexture[i];
			m_aHighLightTexture[i] = nullptr;
		}
	}
}

void MForwardHDRWork::InitializeRenderPass()
{
	if (!m_pTempRenderTarget)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderPass error: rt == nullptr");
		return;
	}

	//Init RenderPass
	m_pTempRenderPass = new MRenderPass();
	m_pTempRenderPass->m_vBackDesc.push_back(MRenderPass::MTargetDesc());
	m_pTempRenderPass->m_vBackDesc.back().bClearWhenRender = true;
	m_pTempRenderPass->m_vBackDesc.back().cClearColor = m_pRenderProgram->GetClearColor();
	m_pTempRenderPass->m_vBackDesc.push_back(MRenderPass::MTargetDesc());
	m_pTempRenderPass->m_vBackDesc.back().bClearWhenRender = true;
	m_pTempRenderPass->m_vBackDesc.back().cClearColor = MColor::Black_T;
}

void MForwardHDRWork::ReleaseRenderPass()
{
	if (m_pTempRenderPass)
	{
		GetEngine()->GetDevice()->DestroyRenderPass(m_pTempRenderPass);
		delete m_pTempRenderPass;
		m_pTempRenderPass = nullptr;
	}
}

void MForwardHDRWork::CheckRenderTargetSize(const Vector2& v2ViewportSize)
{
	if (m_pTempRenderTarget)
	{
		Vector2 v2Size = m_pTempRenderTarget->GetSize();
		if (v2Size.x != v2ViewportSize.x || v2Size.y != v2ViewportSize.y)
		{
			for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
			{
				m_aBackTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aBackTexture[i]->SetSize(v2ViewportSize);
				m_aBackTexture[i]->GenerateBuffer(GetEngine()->GetDevice());

				m_aHighLightTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aHighLightTexture[i]->SetSize(v2ViewportSize);
				m_aHighLightTexture[i]->GenerateBuffer(GetEngine()->GetDevice());
			}

			m_pTempRenderTarget->Resize(v2ViewportSize);
		}
	}
}

void MForwardHDRWork::RenderBloom(MPostProcessRenderInfo& info)
{
	if (!m_pGaussianBlurWork)
		return;

	m_pGaussianBlurWork->CheckRenderTargetSize(info.pViewport->GetSize());

	info.pPrevLevelOutput = m_aHighLightTexture[info.unFrameIndex];
	m_pGaussianBlurWork->Render(info);
}

void MForwardHDRWork::RenderCombine(MPostProcessRenderInfo& info)
{
	if (!m_pCombineWork || !m_pGaussianBlurWork)
		return;

	m_pCombineWork->CheckRenderTargetSize(info.pViewport->GetSize());

	info.pPrevLevelOutput = m_pGaussianBlurWork->GetRenderTarget()->GetBackTexture(info.unFrameIndex)->at(0);
	info.pPrevLevelOutput1 = m_aBackTexture[info.unFrameIndex];
	

	m_pCombineWork->Render(info);
}
