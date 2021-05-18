#include "MForwardRenderProgram.h"

#include "MNode.h"
#include "MScene.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MRenderGraph.h"

#include "MMaterialGroup.h"
#include "Material/MMaterialResource.h"

#include "MForwardRenderWork.h"
#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"
#include "MForwardDebugRenderWork.h"

#include "MIRenderTarget.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MRenderableMeshComponent.h"

#include <algorithm>
#include <float.h>

M_OBJECT_IMPLEMENT(MForwardRenderProgram, MIRenderProgram)

MForwardRenderProgram::MForwardRenderProgram()
	: MIRenderProgram()
	, m_RenderInfo()
	, m_pShadowMapWork(nullptr)
	, m_pRenderWork(nullptr)
	, m_pTransparentWork(nullptr)
	, m_pRenderGraph(nullptr)
	, m_cClearColor(MColor::Black_T)
{
	
}

MForwardRenderProgram::~MForwardRenderProgram()
{
}

void MForwardRenderProgram::Initialize()
{

	m_pRenderGraph = new MRenderGraph(GetEngine());

	m_pShadowMapWork = GetEngine()->GetObjectManager()->CreateObject<MForwardShadowMapWork>();
	m_pShadowMapWork->Initialize(this);

	m_pRenderWork = GetEngine()->GetObjectManager()->CreateObject<MForwardRenderWork>();
	m_pRenderWork->Initialize(this);

 	m_pTransparentWork = GetEngine()->GetObjectManager()->CreateObject<MForwardTransparentWork>();
 	m_pTransparentWork->Initialize(this);

	m_pDebugWork = GetEngine()->GetObjectManager()->CreateObject<MForwardDebugRenderWork>();
	m_pDebugWork->Initialize(this);

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

	if (m_pDebugWork)
	{
		m_pDebugWork->DeleteLater();
		m_pDebugWork = nullptr;
	}

	if (m_pRenderGraph)
	{
		m_pRenderGraph->Release();
		delete m_pRenderGraph;
		m_pRenderGraph = nullptr;
	}

}

void MForwardRenderProgram::Render(MIRenderer* pRenderer, MViewport* pViewport, MRenderCommand* pCommand)
{
	if (!pViewport)
		return;

	MNode* pCameraNode = pViewport->GetCamera();
	MScene* pScene = pViewport->GetScene();

	MRenderInfo& info = *GetRenderInfo();

	info = MRenderInfo();

	info.fDelta = m_pEngine->GetInstantDelta();
	info.unFrameIndex = pCommand->m_unFrameIdx;
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCameraComponent = pCameraNode->GetComponent<MCameraComponent>();
	info.pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>();
	info.pDirectionalLightComponent = pScene->FindActiveDirectionLight();
	info.pScene = pViewport->GetScene();
	info.pPrimaryCommand = pCommand;

	if (!info.pCameraComponent || !info.pCameraSceneComponent)
		return;

	Render(info);
}

void MForwardRenderProgram::Render(MRenderInfo& info)
{
	info.pViewport->LockMatrix();

	GenerateRenderGroup(info);
	
	if (m_pRenderGraph->GetCompiled() || m_pRenderGraph->Compile(GetEngine()->GetDevice()))
	{
		m_pRenderGraph->Render();
	}

	info.pViewport->UnlockMatrix();
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
// 	if (m_pRenderWork)
// 	{
// 		m_pRenderWork->SetClearColor(cClearColor);
// 	}
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

		if(!pRenderGroup)
			continue;

		pRenderGroup->m_pMaterial = pMaterialGroup->m_pMaterial;

		for (MRenderableMeshComponent* pMeshComponent : pMaterialGroup->m_vMeshComponents)
		{
			MNode* pNode = pMeshComponent->GetOwnerNode();
			if(!pNode)
				continue;

			if (!pNode->GetVisibleRecursively())
				continue;

			if (MCameraFrustum::EOUTSIDE == info.pViewport->GetCameraFrustum()->ContainTest(*pMeshComponent->GetBoundsAABB()))
				continue;

			pRenderGroup->m_vMeshComponents.push_back(pMeshComponent);

			const MBoundsAABB* pBounds = pMeshComponent->GetBoundsAABB();
			pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
		}
	}

	info.cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}
