#include "MForwardRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MTexture.h"
#include "MViewport.h"

#include "MMaterialGroup.h"
#include "Model/MModelInstance.h"
#include "Model/MIModelMeshInstance.h"
#include "Material/MMaterialResource.h"

#include "MForwardRenderWork.h"
#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"

#include "MIRenderTarget.h"

#include <algorithm>


M_OBJECT_IMPLEMENT(MForwardRenderProgram, MIRenderProgram)


MForwardRenderProgram::MForwardRenderProgram()
	: MIRenderProgram()
	, m_pShadowMapWork(nullptr)
	, m_pRenderWork(nullptr)
	, m_pTransparentWork(nullptr)
	, m_cClearColor(MColor::Black_T)
{
	
}

MForwardRenderProgram::~MForwardRenderProgram()
{
}

void MForwardRenderProgram::Initialize()
{
	m_pShadowMapWork = GetEngine()->GetObjectManager()->CreateObject<MForwardShadowMapWork>();
	m_pShadowMapWork->Initialize(this);

	m_pRenderWork = GetEngine()->GetObjectManager()->CreateObject<MForwardRenderWork>();
	m_pRenderWork->Initialize(this);

	m_pTransparentWork = GetEngine()->GetObjectManager()->CreateObject<MForwardTransparentWork>();
	m_pTransparentWork->Initialize(this);

}

void MForwardRenderProgram::Release()
{
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->DeleteLater();
		m_pShadowMapWork = nullptr;
	}

	if (m_pRenderWork)
	{
		m_pRenderWork->DeleteLater();
		m_pRenderWork = nullptr;
	}

	if (m_pTransparentWork)
	{
		m_pTransparentWork->DeleteLater();
		m_pTransparentWork = nullptr;
	}

}

void MForwardRenderProgram::Render(MIRenderer* pRenderer, const std::vector<MViewport*>& vViewports)
{
	if (vViewports.empty())
		return;

	//Only one viewport.
	MViewport* pViewport = vViewports[0];

	MRenderInfo info;
	memset(&info, 0, sizeof(MRenderInfo));

	info.unFrameIndex = pRenderer->GetFrameIndex();
	info.pRenderTarget = GetRenderTarget();
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCamera = pViewport->GetCamera();
	info.pScene = pViewport->GetScene();

	pViewport->LockMatrix();

	GenerateRenderGroup(info);

	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->Render(info);
		info.pShadowMapTexture = m_pShadowMapWork->GetShadowMap(info.unFrameIndex);
	}

	if (m_pRenderWork)
	{
		m_pRenderWork->Render(info);
	}

	if (m_pTransparentWork)
	{
		m_pTransparentWork->Render(info);
	}

	pViewport->UnlockMatrix();
}

void MForwardRenderProgram::OnCreated()
{
	Super::OnCreated();
}

void MForwardRenderProgram::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MForwardRenderProgram::SetClearColor(const MColor& cClearColor)
{
	m_cClearColor = cClearColor;
	if (m_pRenderWork)
	{
		m_pRenderWork->SetClearColor(cClearColor);
	}
}

void MForwardRenderProgram::GenerateRenderGroup(MRenderInfo& info)
{
	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::vector<MMaterialGroup*>& matGroups = info.pScene->GetMaterialGroup();
	for (MMaterialGroup* pMaterialGroup : matGroups)
	{
		MMaterialGroup* pRenderGroup = nullptr;

		if (pMaterialGroup->m_pMaterial->GetMaterialType() == MEMaterialType::EDefault)
		{
			info.vMaterialRenderGroup.push_back(MMaterialGroup());
			pRenderGroup = &info.vMaterialRenderGroup.back();
		}
		else if (pMaterialGroup->m_pMaterial->GetMaterialType() == MEMaterialType::EDepthPeel)
		{
			info.vTransparentRenderGroup.push_back(MMaterialGroup());
			pRenderGroup = &info.vTransparentRenderGroup.back();
		}


		pRenderGroup->m_pMaterial = pMaterialGroup->m_pMaterial;

		for (MIMeshInstance* pMeshIns : pMaterialGroup->m_vMeshInstances)
		{
			if (!pMeshIns->GetVisibleRecursively())
				continue;

			if (MCameraFrustum::EOUTSIDE == info.pViewport->GetCameraFrustum()->ContainTest(*pMeshIns->GetBoundsAABB()))
				continue;

			pRenderGroup->m_vMeshInstances.push_back(pMeshIns);

			const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
			pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
		}
	}

	info.cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}
