#include "MDeferredRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MRenderGraph.h"

#include "MMaterialGroup.h"
#include "Model/MModelInstance.h"
#include "Model/MIModelMeshInstance.h"
#include "Material/MMaterialResource.h"

#include "MDeferredGBufferWork.h"
#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"

#include "MIRenderTarget.h"

#include <algorithm>
#include <float.h>

M_OBJECT_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)

MDeferredRenderProgram::MDeferredRenderProgram()
	: MIRenderProgram()
	, m_RenderInfo()
	, m_pShadowMapWork(nullptr)
	, m_pRenderWork(nullptr)
	, m_pTransparentWork(nullptr)
	, m_pRenderGraph(nullptr)
	, m_cClearColor(MColor::Black_T)
{
	
}

MDeferredRenderProgram::~MDeferredRenderProgram()
{
}

void MDeferredRenderProgram::Initialize()
{

	m_pRenderGraph = new MRenderGraph(GetEngine());

	m_pShadowMapWork = GetEngine()->GetObjectManager()->CreateObject<MForwardShadowMapWork>();
	m_pShadowMapWork->Initialize(this);

	m_pRenderWork = GetEngine()->GetObjectManager()->CreateObject<MDeferredGBufferWork>();
	m_pRenderWork->Initialize(this);

	m_pTransparentWork = GetEngine()->GetObjectManager()->CreateObject<MForwardTransparentWork>();
	m_pTransparentWork->Initialize(this);

}

void MDeferredRenderProgram::Release()
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

	if (m_pRenderGraph)
	{
		m_pRenderGraph->Release();
		delete m_pRenderGraph;
		m_pRenderGraph = nullptr;
	}

}

void MDeferredRenderProgram::Render(MIRenderer* pRenderer, MViewport* pViewport, MRenderCommand* pCommand)
{
	if (!pViewport)
		return;

	MRenderInfo& info = GetRenderInfo();

	info = MRenderInfo();

	info.fDelta = m_pEngine->GetInstantDelta();
	info.unFrameIndex = pCommand->m_unFrameIdx;
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCamera = pViewport->GetCamera();
	info.pScene = pViewport->GetScene();
	info.pPrimaryCommand = pCommand;


	Render(info);
}

void MDeferredRenderProgram::Render(MRenderInfo& info)
{
	info.pViewport->LockMatrix();

	GenerateRenderGroup(info);
	
	if (m_pRenderGraph->GetCompiled() || m_pRenderGraph->Compile(GetEngine()->GetDevice()))
	{
		m_pRenderGraph->Render();
	}

	info.pViewport->UnlockMatrix();
}

void MDeferredRenderProgram::OnCreated()
{
	Super::OnCreated();
}

void MDeferredRenderProgram::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MDeferredRenderProgram::SetClearColor(const MColor& cClearColor)
{
	m_cClearColor = cClearColor;
// 	if (m_pRenderWork)
// 	{
// 		m_pRenderWork->SetClearColor(cClearColor);
// 	}
}

void MDeferredRenderProgram::GenerateRenderGroup(MRenderInfo& info)
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

		if(!pRenderGroup)
			continue;

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
