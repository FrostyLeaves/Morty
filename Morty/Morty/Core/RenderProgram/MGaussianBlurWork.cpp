#include "MGaussianBlurWork.h"
#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MGaussianBlurWork, MIPostProcessWork)

MGaussianBlurWork::MGaussianBlurWork()
    : MIPostProcessWork()
    , m_aMaterial()
	, m_fBlurRadius(1.0f)
	, m_unIteration(6)
{
}

MGaussianBlurWork::~MGaussianBlurWork()
{
}

void MGaussianBlurWork::Render(MPostProcessRenderInfo& info)
{
	UpdateShaderSharedParams(info);

	for (uint32_t i = 0; i < 2 * m_unIteration; ++i)
	{
		MMaterial* pMaterial = nullptr;
		if (MShaderParamSet* pMaterialParamSet = m_aMaterial[i % 2]->GetMaterialParamSet())
		{
			if (i == 0)
			{
				MIRenderBackTexture* pBackTexture = info.pPrevLevelOutput;
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[2];
			}
			else if (i % 2 == 0)
			{
				MIRenderBackTexture* pBackTexture = m_aTempRenderTarget[1]->GetBackTexture(info.unFrameIndex)->at(0);
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[0];
			}
			else
			{
				MIRenderBackTexture* pBackTexture = m_aTempRenderTarget[0]->GetBackTexture(info.unFrameIndex)->at(0);
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[1];
			}

		}

		info.pRenderer->BeginRenderPass(m_pTempRenderPass, m_aTempRenderTarget[i % 2]);

		Vector2 v2LeftTop = info.pViewport->GetLeftTop();
		Vector2 v2ViewportSize = info.pViewport->GetSize();
		info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, v2ViewportSize.x, v2ViewportSize.y, 0.0f, 1.0f);

		if (info.pRenderer->SetUseMaterial(pMaterial))
		{
			info.pRenderer->DrawMesh(m_pScreenDrawMesh);
		}

		info.pRenderer->EndRenderPass();
	}
}

void MGaussianBlurWork::UpdateShaderSharedParams(MPostProcessRenderInfo& info)
{
	Vector2 v2ViewportSize = info.pViewport->GetSize();
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[0]->GetMaterialParamSet())
	{
		MIRenderBackTexture* pBackTexture = m_aTempRenderTarget[1]->GetBackTexture(info.unFrameIndex)->at(0);
		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.x = m_fBlurRadius / v2ViewportSize.x;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[1]->GetMaterialParamSet())
	{
		MIRenderBackTexture* pBackTexture = m_aTempRenderTarget[0]->GetBackTexture(info.unFrameIndex)->at(0);
		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.y = m_fBlurRadius / v2ViewportSize.y;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[2]->GetMaterialParamSet())
	{
		MIRenderBackTexture* pBackTexture = info.pPrevLevelOutput;
		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.x = m_fBlurRadius / v2ViewportSize.x;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}

}

void MGaussianBlurWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MGaussianBlurWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MGaussianBlurWork::InitializeRenderTargets()
{
	for (uint32_t nRtIdx = 0; nRtIdx < 2; ++nRtIdx)
	{
		m_aTempRenderTarget[nRtIdx] = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

		for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
		{
			MRenderBackTexture* pBackTexture = new MRenderBackTexture();
			m_aBackTexture[nRtIdx][i] = pBackTexture;
		}

		m_aTempRenderTarget[nRtIdx]->SetBackTexture(m_aBackTexture[nRtIdx], 0);
	}
}

void MGaussianBlurWork::ReleaseRenderTargets()
{
	for (uint32_t nRtIdx = 0; nRtIdx < 2; ++nRtIdx)
	{
		if (m_aTempRenderTarget[nRtIdx])
		{
			m_aTempRenderTarget[nRtIdx]->DeleteLater();
			m_aTempRenderTarget[nRtIdx] = nullptr;
		}

		for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
		{
			if (m_aBackTexture[nRtIdx][i])
			{
				m_aBackTexture[nRtIdx][i]->DestroyBuffer(GetEngine()->GetDevice());
				delete m_aBackTexture[nRtIdx][i];
				m_aBackTexture[nRtIdx][i] = nullptr;
			}
		}
	}
}

void MGaussianBlurWork::InitializeRenderPass()
{

	//Init RenderPass
	m_pTempRenderPass = new MRenderPass();
	m_pTempRenderPass->m_vBackDesc.push_back(MPassTargetDescription());
	m_pTempRenderPass->m_vBackDesc.back().bClearWhenRender = true;
	m_pTempRenderPass->m_vBackDesc.back().cClearColor = m_pRenderProgram->GetClearColor();

	m_pTempRenderPass->m_DepthDesc.bClearWhenRender = true;
}

void MGaussianBlurWork::ReleaseRenderPass()
{
	if (m_pTempRenderPass)
	{
		GetEngine()->GetDevice()->DestroyRenderPass(m_pTempRenderPass);
		delete m_pTempRenderPass;
		m_pTempRenderPass = nullptr;
	}
}

void MGaussianBlurWork::CheckRenderTargetSize(const Vector2& v2ViewportSize)
{
	for (uint32_t nRtIdx = 0; nRtIdx < 2; ++nRtIdx)
	{
		Vector2 v2Size = m_aTempRenderTarget[nRtIdx]->GetSize();
		if (v2Size.x != v2ViewportSize.x || v2Size.y != v2ViewportSize.y)
		{
			for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
			{
				m_aBackTexture[nRtIdx][i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aBackTexture[nRtIdx][i]->SetSize(v2ViewportSize);
				m_aBackTexture[nRtIdx][i]->GenerateBuffer(GetEngine()->GetDevice());
			}

			m_aTempRenderTarget[nRtIdx]->Resize(v2ViewportSize);
		}
	}
}

void MGaussianBlurWork::Initialize(MIRenderProgram* pRenderProgram)
{
	Super::Initialize(pRenderProgram);

	m_pRenderProgram = pRenderProgram;

	InitializeMesh();
	InitializeMaterial();
	InitializeRenderTargets();
	InitializeRenderPass();
}

void MGaussianBlurWork::Release()
{
	ReleaseRenderPass();
	ReleaseRenderTargets();
	ReleaseMaterial();
	ReleaseMesh();

	Super::Release();
}

MTextureRenderTarget* MGaussianBlurWork::GetRenderTarget()
{
	return m_aTempRenderTarget[1];
}

void MGaussianBlurWork::InitializeMaterial()
{
	MResource* pVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/gaussian_blur.mvs");
	MResource* pPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/gaussian_blur.mps");

	for (uint32_t i = 0; i < 3; ++i)
	{
		m_aMaterial[i] = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();

		m_aMaterial[i]->LoadVertexShader(pVSResource);
		m_aMaterial[i]->LoadPixelShader(pPSResource);

		m_aMaterial[i]->AddRef();
	}
}

void MGaussianBlurWork::ReleaseMaterial()
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (m_aMaterial[i])
		{
			m_aMaterial[i]->SubRef();
			m_aMaterial[i] = nullptr;
		}
	}
}
