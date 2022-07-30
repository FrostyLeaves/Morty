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

	std::shared_ptr<MResource> pVertexShader = pResourceSystem->LoadResource("./Shader/shadowmap.mvs");
	std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("./Shader/shadowmap.mps");
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

	MTexture* pShadowMapTexture = MTexture::CreateShadowMap();
	pShadowMapTexture->SetSize(Vector2(1024.0, 1024.0));
	pShadowMapTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_renderPass.SetDepthTexture(pShadowMapTexture, { true, MColor::White });
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


	if (MTexture* pDepthTexture = m_renderPass.GetDepthTexture())
	{
		pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete pDepthTexture;
		pDepthTexture = nullptr;
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

	pCommand->BeginRenderPass(&m_renderPass);

	DrawShadowMesh(info, pCommand);

	pCommand->EndRenderPass();
}

void MShadowMapRenderWork::DrawShadowMesh(MRenderInfo& info, MIRenderCommand* pCommand)
{
	MTexture* pShadowmap = GetShadowMap();
	
	size_t nCascadedSize = info.cCascadedShadow.size();
	if (nCascadedSize == 0)
	{
		return;
	}

	size_t nRowSize = size_t(ceil(sqrtf(float(nCascadedSize))));
	size_t nCascadedIdx = 0;

	Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
	Vector2 v2Size = pShadowmap->GetSize();
	Vector2 v2ScascadedTextureSize = Vector2(v2Size.x / nRowSize, v2Size.y / nRowSize);

	for (size_t nRowIdx = 0; nRowIdx < nRowSize; ++nRowIdx)
	{
		for (size_t nColIdx = 0; nColIdx < nRowSize; ++nColIdx)
		{
			MCascadedShadowInfo& csInfo = info.cCascadedShadow[nCascadedIdx];

			float x = v2LeftTop.x + v2ScascadedTextureSize.x * nRowIdx;
			float y = v2LeftTop.y + v2ScascadedTextureSize.y * nColIdx;
			float w = v2ScascadedTextureSize.x;
			float h = v2ScascadedTextureSize.y;

			pCommand->SetViewport(MViewportInfo(x, y, w, h));
			pCommand->SetScissor(MScissorInfo(x, y, w, h));

			auto& materialGroup = csInfo.m_tShadowGroupMesh;
			for (auto& pr : materialGroup)
			{
				if (std::shared_ptr<MSkeletonInstance> pSkeletonIns = pr.first)
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

			if (++nCascadedIdx >= nCascadedSize)
			{
				break;
			}
		}

		if (nCascadedIdx >= nCascadedSize)
		{
			break;
		}
	}

	

}

void MShadowMapRenderWork::UpdateShadowParams(MRenderInfo& info)
{
	m_shadowParamSet.UpdateShaderSharedParams(info);
}

MTexture* MShadowMapRenderWork::GetShadowMap()
{
	return m_renderPass.GetDepthTexture();
}

void MShadowMapRenderWork::CollectShadowMesh(MRenderInfo& info)
{
	MViewport* pViewport = info.pViewport;

	if (!pViewport)
		return;

	MScene* pScene = pViewport->GetScene();
	if (!pScene)
		return;

	if (!info.pDirectionalLightEntity)
	{
		info.pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();

		if (!info.pDirectionalLightEntity)
			return;
	}

	MSceneComponent* pLightSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
		return;

	MComponentGroup<MRenderableMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderableMeshComponent>();
	if (!pMeshComponentGroup)
		return;

	Vector3 v3LightDir = pLightSceneComponent->GetForward();

	struct CascadedData
	{
		bool bGenerateShadow = false;
		Vector3 v3ShadowMin = Vector3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
		Vector3 v3ShadowMax = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	} vCascadedData[MRenderGlobal::CASCADED_SHADOW_MAP_NUM];

	MCameraFrustum cameraFrustum = pViewport->GetCameraFrustum();
	std::vector<MCameraFrustum>&& vCutFrustum = cameraFrustum.CutFrustum(std::vector<float>(MRenderGlobal::CASCADED_SHADOW_MAP_NUM, 1.0f / MRenderGlobal::CASCADED_SHADOW_MAP_NUM));

	for (MRenderableMeshComponent& component : pMeshComponentGroup->m_vComponents)
	{
		if (!component.IsValid())
			continue;

		if (std::shared_ptr<MMaterial> pMaterial = component.GetMaterial())
		{
			if (pMaterial->GetMaterialType() != MEMaterialType::EDefault && pMaterial->GetMaterialType() != MEMaterialType::EDeferred)
				continue;
		}

		MEntity* pEntity = component.GetEntity();

		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

		if (pSceneComponent->GetVisibleRecursively() && component.GetGenerateDirLightShadow())
		{
			std::shared_ptr<MSkeletonInstance> pSkeletonInstance = component.GetSkeletonInstance();

			if (const MBoundsAABB* pBounds = component.GetBoundsAABB())
			{
				for (size_t nCascadedIdx = 0; nCascadedIdx < vCutFrustum.size(); ++nCascadedIdx)
				{
					CascadedData& cd = vCascadedData[nCascadedIdx];

					if (vCutFrustum[nCascadedIdx].ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
					{
						auto& vMeshes = info.cCascadedShadow[nCascadedIdx].m_tShadowGroupMesh[pSkeletonInstance];

						vMeshes.push_back(&component);
						pBounds->UnionMinMax(cd.v3ShadowMin, cd.v3ShadowMax);
						cd.bGenerateShadow = true;
					}
				}
			}
		}
	}

	for (size_t nCascadedIdx = 0; nCascadedIdx < vCutFrustum.size(); ++nCascadedIdx)
	{
		CascadedData& cd = vCascadedData[nCascadedIdx];
		MBoundsAABB cGenerateShadowRenderAABB;

		if (cd.bGenerateShadow)
		{
			cGenerateShadowRenderAABB.SetMinMax(cd.v3ShadowMin, cd.v3ShadowMax);
		}
		else
		{
			cGenerateShadowRenderAABB = info.cCaclSceneRenderAABB;
		}
		info.cCascadedShadow[nCascadedIdx].m4DirLightInvProj = pViewport->GetLightInverseProjection(info.pDirectionalLightEntity, info.cCaclSceneRenderAABB, cGenerateShadowRenderAABB);
	}
}

