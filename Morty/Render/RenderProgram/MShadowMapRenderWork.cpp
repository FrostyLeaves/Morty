#include "MShadowMapRenderWork.h"

#include "MScene.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MSkeleton.h"
#include "MMaterial.h"
#include "MRenderPass.h"
#include "MRenderCommand.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MSceneComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"


MORTY_CLASS_IMPLEMENT(MShadowMapRenderWork, MObject)

MShadowMapRenderWork::MShadowMapRenderWork()
	: MObject()
	, m_pShadowStaticMaterial(nullptr)
	, m_pShadowSkeletonMaterial(nullptr)
	, m_shadowParamSet()
	, m_renderPass()
{

}

MShadowMapRenderWork::~MShadowMapRenderWork()
{

}

void MShadowMapRenderWork::OnCreated()
{
	Super::OnCreated();

	m_shadowParamSet.InitializeShaderParamSet(GetEngine());

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pShadowStaticMaterial = pResourceSystem->CreateResource<MMaterial>("Shadow Material");

	MResource* pVertexShader = pResourceSystem->LoadResource("./Shader/shadowmap.mvs");
	MResource* pPixelShader = pResourceSystem->LoadResource("./Shader/shadowmap.mps");
	m_pShadowStaticMaterial->LoadVertexShader(pVertexShader);
	m_pShadowStaticMaterial->LoadPixelShader(pPixelShader);
	m_pShadowStaticMaterial->SetRasterizerType(MERasterizerType::ECullFront);
	m_pShadowStaticMaterial->AddRef();

	m_pShadowSkeletonMaterial = pResourceSystem->CreateResource<MMaterial>("Shadow Material With Skeleton");
	m_pShadowSkeletonMaterial->GetShaderMacro()->SetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE, "1");
	m_pShadowSkeletonMaterial->LoadVertexShader(pVertexShader);
	m_pShadowSkeletonMaterial->LoadPixelShader(pPixelShader);
	m_pShadowSkeletonMaterial->SetRasterizerType(MERasterizerType::ECullFront);
	m_pShadowSkeletonMaterial->AddRef();

	m_renderPass.m_pDepthTexture = MTexture::CreateShadowMap();
	m_renderPass.m_pDepthTexture->SetSize(Vector2(1024.0, 1024.0));
	m_renderPass.m_pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_renderPass.m_DepthDesc.bClearWhenRender = true;
	m_renderPass.m_DepthDesc.cClearColor = MColor::White;
	m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MShadowMapRenderWork::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	Super::OnDelete();

	m_shadowParamSet.ReleaseShaderParamSet(GetEngine());

	m_pShadowStaticMaterial->SubRef();
	m_pShadowStaticMaterial = nullptr;

	m_pShadowSkeletonMaterial->SubRef();
	m_pShadowSkeletonMaterial = nullptr;


	if (m_renderPass.m_pDepthTexture)
	{
		m_renderPass.m_pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_renderPass.m_pDepthTexture;
		m_renderPass.m_pDepthTexture = nullptr;
	}

	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MShadowMapRenderWork::RenderShadow(MRenderInfo& info)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
		return;

	MViewport* pViewport = info.pViewport;
	MTexture* pShadowmap = GetShadowMap();

	pCommand->BeginRenderPass(&m_renderPass);

	Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
	Vector2 v2Size = pShadowmap->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

	DrawShadowMesh(info, pCommand);

	pCommand->EndRenderPass();
}

void MShadowMapRenderWork::DrawShadowMesh(MRenderInfo& info, MIRenderCommand* pCommand)
{


	auto& materialGroup = info.m_tShadowGroupMesh;
	for (auto& pr : materialGroup)
	{
		if (MSkeletonInstance* pSkeletonIns = pr.first)
		{
			pCommand->SetUseMaterial(m_pShadowSkeletonMaterial);
			pCommand->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
		}
		else
		{
			pCommand->SetUseMaterial(m_pShadowStaticMaterial);
		}

		pCommand->SetShaderParamSet(&m_shadowParamSet);

		std::vector<MRenderableMeshComponent*>& vMesh = pr.second;

		for (MRenderableMeshComponent* pMeshComponent : vMesh)
		{
			pCommand->SetShaderParamSet(pMeshComponent->GetShaderMeshParamSet());
			pCommand->DrawMesh(pMeshComponent->GetMesh());
		}
	}

}

void MShadowMapRenderWork::UpdateShadowParams(MRenderInfo& info)
{
	m_shadowParamSet.UpdateShaderSharedParams(info);
}

void MShadowMapRenderWork::UpdateShadowRenderGroup(MRenderInfo& info)
{
	if (!info.pViewport)
		return;

	MScene* pScene = info.pViewport->GetScene();
	if (!pScene)
		return;

	if (!info.pDirectionalLightEntity)
	{
		info.pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();

		if(!info.pDirectionalLightEntity)
			return;
	}

	MSceneComponent* pLightSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
		return;

	MComponentGroup<MRenderableMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderableMeshComponent>();
	if (!pMeshComponentGroup)
		return;

	Vector3 v3LightDir = pLightSceneComponent->GetForward();

	bool bGenerateShadow = false;
	Vector3 v3ShadowMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3ShadowMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::map<MSkeletonInstance*, size_t> tSkeletonToIndex;


	for (MRenderableMeshComponent& component : pMeshComponentGroup->m_vComponent)
	{
		if (!component.IsValid())
			continue;

		if (MMaterial* pMaterial = component.GetMaterial())
		{
			if(pMaterial->GetMaterialType() != MEMaterialType::EDefault)
				continue;
		}

		MEntity* pEntity = component.GetEntity();

		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

		if (pSceneComponent->GetVisibleRecursively() && component.GetGenerateDirLightShadow())
		{
			MSkeletonInstance* pSkeletonInstance = component.GetSkeletonInstance();

			auto& vMeshes = info.m_tShadowGroupMesh[pSkeletonInstance];


			if (const MBoundsAABB* pBounds = component.GetBoundsAABB())
			{
				if (info.pViewport->GetCameraFrustum().ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
				{
					vMeshes.push_back(&component);
					pBounds->UnionMinMax(v3ShadowMin, v3ShadowMax);

					bGenerateShadow = true;
				}
			}
		}
	}

	if (bGenerateShadow)
	{
		info.cShadowRenderAABB.SetMinMax(v3ShadowMin, v3ShadowMax);
	}
	else
	{
		info.cShadowRenderAABB = info.cMeshRenderAABB;
	}

	info.m4DirLightInvProj = info.pViewport->GetLightInverseProjection(info.pDirectionalLightEntity, info.cMeshRenderAABB, info.cShadowRenderAABB);
}

MTexture* MShadowMapRenderWork::GetShadowMap()
{
	return m_renderPass.m_pDepthTexture;
}
