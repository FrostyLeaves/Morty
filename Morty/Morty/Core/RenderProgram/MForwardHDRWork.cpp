#include "MForwardHDRWork.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardHDRWork, MObject)

MForwardHDRWork::MForwardHDRWork()
    : MObject()
	, m_pRenderProgram(nullptr)
	, m_aBackTexture()
	, m_aDepthTexture()
	, m_pTempRenderTarget(nullptr)
	, m_HDRRenderPass()
	, m_ScreenDrawMesh()
	, m_pHDRMaterial(nullptr)
{
}

MForwardHDRWork::~MForwardHDRWork()
{
}

void MForwardHDRWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeMesh();
	InitializeMaterial();
	InitializeRenderTargets();
	InitializeRenderPass();
}

void MForwardHDRWork::Release()
{
	ReleaseRenderPass();
	ReleaseRenderTargets();
	ReleaseMaterial();
	ReleaseMesh();
}

MIRenderTarget* MForwardHDRWork::GetTempRenderTarget()
{
	return m_pTempRenderTarget;
}

void MForwardHDRWork::CheckTextureSize(MRenderInfo& info)
{
	if (m_pTempRenderTarget)
	{
		Vector2 v2Size = m_pTempRenderTarget->GetSize();
		Vector2 v2ViewportSize = info.pViewport->GetSize();
		if (v2Size.x != v2ViewportSize.x || v2Size.y != v2ViewportSize.y)
		{
			for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
			{
				m_aBackTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aBackTexture[i]->SetSize(v2ViewportSize);
				m_aBackTexture[i]->GenerateBuffer(GetEngine()->GetDevice());

				m_aDepthTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aDepthTexture[i]->SetSize(v2ViewportSize);
				m_aDepthTexture[i]->GenerateBuffer(GetEngine()->GetDevice());
			}

			m_pTempRenderTarget->Resize(v2ViewportSize);
		}
	}
}

void MForwardHDRWork::Render(MRenderInfo& info)
{
	info.pRenderer->BeginRenderPass(&m_HDRRenderPass, m_pRenderProgram->GetRenderTarget());

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pHDRMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = m_pTempRenderTarget->GetBackTexture(info.unFrameIndex)->at(0);
		pMaterialParamSet->m_vTextures[0]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(m_pHDRMaterial))
	{
		info.pRenderer->DrawMesh(&m_ScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass();
}

void MForwardHDRWork::InitializeMesh()
{
	MMesh<Vector2>& mesh = m_ScreenDrawMesh;
	mesh.ResizeVertices(4);
	Vector2* vVertices = (Vector2*)mesh.GetVertices();

	vVertices[0] = Vector2(-1, -1);
	vVertices[1] = Vector2(1, -1);
	vVertices[2] = Vector2(-1, 1);
	vVertices[3] = Vector2(1, 1);

	mesh.ResizeIndices(2, 3);
	uint32_t* vIndices = mesh.GetIndices();

	vIndices[0] = 0;
	vIndices[1] = 2;
	vIndices[2] = 1;

	vIndices[3] = 2;
	vIndices[4] = 3;
	vIndices[5] = 1;
}

void MForwardHDRWork::ReleaseMesh()
{
	m_ScreenDrawMesh.DestroyBuffer(m_pEngine->GetDevice());
}

void MForwardHDRWork::InitializeRenderTargets()
{
	m_pTempRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MRenderBackTexture* pBackTexture = new MRenderBackTexture();
//		pBackTexture->SetType(METextureLayout::ERGBA16);
		pBackTexture->SetSize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
		pBackTexture->GenerateBuffer(m_pEngine->GetDevice());

		MRenderDepthTexture* pDepthTexture = new MRenderDepthTexture();
		pDepthTexture->SetSize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
		pDepthTexture->GenerateBuffer(m_pEngine->GetDevice());

		m_aBackTexture[i] = pBackTexture;
		m_aDepthTexture[i] = pDepthTexture;
	}

	m_pTempRenderTarget->SetBackTexture(m_aBackTexture, 0);
	m_pTempRenderTarget->SetDepthTexture(m_aDepthTexture);

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
		if (m_aDepthTexture[i])
		{
			m_aDepthTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			delete m_aDepthTexture[i];
			m_aDepthTexture[i] = nullptr;
		}
	}
}

void MForwardHDRWork::InitializeRenderPass()
{
	if (!m_pRenderProgram->GetRenderTarget())
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderPass error: rt == nullptr");
		return;
	}

	//Init RenderPass
	m_HDRRenderPass.m_vBackDesc.push_back(MRenderPass::MTargetDesc());
	m_HDRRenderPass.m_vBackDesc.back().bClearWhenRender = true;
	m_HDRRenderPass.m_vBackDesc.back().cClearColor = m_pRenderProgram->GetClearColor();

	m_HDRRenderPass.m_DepthDesc.bClearWhenRender = true;
}

void MForwardHDRWork::ReleaseRenderPass()
{
	GetEngine()->GetDevice()->DestroyRenderPass(&m_HDRRenderPass);
}

void MForwardHDRWork::InitializeMaterial()
{
	m_pHDRMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();

	MResource* pVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/high_dynamic_range.mvs");
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
