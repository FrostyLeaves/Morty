#include "MForwardShadowMapWork.h"

#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MRenderGraph.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MScene.h"
#include "MSkeleton.h"
#include "Model/MIMeshInstance.h"
#include "Model/MModelInstance.h"
#include "Light/MDirectionalLight.h"

#include "MForwardRenderProgram.h"

#include <float.h>

M_OBJECT_IMPLEMENT(MForwardShadowMapWork, MObject)

MForwardShadowMapWork::MForwardShadowMapWork()
	: MObject()
	, m_pRenderProgram(nullptr)
	, m_FrameParamSet(1)
	, m_pWorldMatrixParam(nullptr)
	, m_pStaticMaterial(nullptr)
	, m_pAnimMaterial(nullptr)
{
	
}

MForwardShadowMapWork::~MForwardShadowMapWork()
{
}

void MForwardShadowMapWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeRenderGraph();
	InitializeRenderTargets();
	InitializeShaderParamSet();
	InitializeMaterial();
}

void MForwardShadowMapWork::Release()
{
	ReleaseMaterial();
	ReleaseShaderParamSet();
	ReleaseRenderTargets();
}

void MForwardShadowMapWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	MRenderGraphNode* pShadowMapNode = pRenderGraph->AddRenderGraphNode("Shadow Map Node");

	MRenderGraphTexture* pShadowMapTexture = pRenderGraph->FindRenderGraphTexture("Shadow Map");
	if (nullptr == pShadowMapTexture)
	{
		pShadowMapTexture = pRenderGraph->AddRenderGraphTexture("Shadow Map");
		pShadowMapTexture->SetUsage(METextureUsage::ERenderDepth);
		pShadowMapTexture->SetLayout(METextureLayout::EDepth);
		pShadowMapTexture->SetSize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
	}

	MRenderGraphNodeOutput* pShadowMapOutput =  pShadowMapNode->AppendOutput();
	pShadowMapOutput->SetClear(true);
	pShadowMapOutput->SetRenderTexture(pShadowMapTexture);

	pShadowMapNode->BindRenderFunction(std::bind(&MForwardShadowMapWork::Render, this, std::placeholders::_1));

}

void MForwardShadowMapWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MForwardShadowMapWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardRenderProgram* pRenderProgram = dynamic_cast<MForwardRenderProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;
	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	MRenderPass* pRenderPass = pGraphNode->GetRenderPass();
	if (!pRenderPass)
		return;

	info.pDirectionalLight = info.pScene->FindActiveDirectionLight();
	if (nullptr == info.pDirectionalLight)
		return;

	std::vector<MShadowRenderGroup> vShadowMeshGroup;

	UpdateRenderInfo(info, vShadowMeshGroup);

	if (vShadowMeshGroup.empty())
		return;

	info.pRenderer->BeginRenderPass(pRenderPass, info.unFrameIndex);

	RenderMesh(info, vShadowMeshGroup);

	info.pRenderer->EndRenderPass();
}

void MForwardShadowMapWork::UpdateRenderInfo(MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup)
{
	Vector3 v3LightDir = info.pDirectionalLight->GetWorldDirection();

	bool bGenerateShadow = false;
	Vector3 v3ShadowMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3ShadowMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	vShadowMeshGroup.push_back(MShadowRenderGroup());

	std::vector<MModelInstance*>& vModels = *info.pScene->GetAllModelInstance();
	for (MModelInstance* pModelIns : vModels)
	{
		if (pModelIns->GetVisibleRecursively() && pModelIns->GetGenerateDirLightShadow())
		{
			MShadowRenderGroup* pGroup = nullptr;
			if (pModelIns->GetSkeleton())
			{
				vShadowMeshGroup.push_back(MShadowRenderGroup());
				pGroup = &vShadowMeshGroup.back();
				pGroup->pSkeletonInstance = pModelIns->GetSkeleton();
			}
			else
			{
				pGroup = &vShadowMeshGroup.front();
			}
			MShadowRenderGroup& group = *pGroup;

			std::vector<MIMeshInstance*> vMeshIns;
			pModelIns->FindChildrenByType(vMeshIns, static_cast<int>(MNode::MENodeChildType::EProtected));

			for (MIMeshInstance* pMeshIns : vMeshIns)
			{
				if (pMeshIns->GetVisible() && pMeshIns->GetGenerateDirLightShadow())
				{
					const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
					if (info.pViewport->GetCameraFrustum()->ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
					{
						group.vMeshInstances.push_back(pMeshIns);
						pBounds->UnionMinMax(v3ShadowMin, v3ShadowMax);

						bGenerateShadow = true;
					}
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

	info.m4DirLightInvProj = info.pViewport->GetLightInverseProjection(info.pDirectionalLight, info.cMeshRenderAABB, info.cShadowRenderAABB);
}

void MForwardShadowMapWork::RenderMesh(MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup)
{
	if (m_pWorldMatrixParam)
	{
		MStruct& cStruct = *m_pWorldMatrixParam->var.GetStruct();
		cStruct[0] = info.pViewport->GetCameraInverseProjection();
		cStruct[1] = info.m4DirLightInvProj;

		m_pWorldMatrixParam->SetDirty();
	}

	info.pRenderer->SetViewport(0.0f, 0.0f, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE, 0.0f, 1.0f);

	MStruct* pWorldStruct = m_pWorldMatrixParam->var.GetStruct();
	(*pWorldStruct)[0] = info.m4DirLightInvProj;
	m_pWorldMatrixParam->SetDirty();

	for (MShadowRenderGroup& group : vShadowMeshGroup)
	{
		if(group.vMeshInstances.empty())
			continue;

		if (MSkeletonInstance* pSkeleton = group.pSkeletonInstance)
		{
			info.pRenderer->SetUseMaterial(m_pAnimMaterial);
			info.pRenderer->SetShaderParamSet(pSkeleton->GetShaderParamSet());
		}
		else
		{
			info.pRenderer->SetUseMaterial(m_pStaticMaterial);
		}

		info.pRenderer->SetShaderParamSet(&m_FrameParamSet);

		for (MIMeshInstance* pMeshIns : group.vMeshInstances)
		{
			info.pRenderer->SetShaderParamSet(pMeshIns->GetShaderMeshParamSet());
			info.pRenderer->DrawMesh(pMeshIns->GetMesh());
		}

	}
}

void MForwardShadowMapWork::InitializeRenderTargets()
{
//	m_pShadowDepthMapRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

// 	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
// 	{
// 		MRenderTexture* pDepthTexture = new MRenderTexture();
// 		pDepthTexture->SetType(METextureLayout::ER32);
// 		pDepthTexture->SetUsage(METextureUsage::ERenderDepth);
// 		pDepthTexture->SetSize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
// 		pDepthTexture->GenerateBuffer(m_pEngine->GetDevice());
// 
// 		m_vShadowDepthTexture[i] = pDepthTexture;
// 	}

//	m_pShadowDepthMapRenderTarget->SetDepthTexture(m_vShadowDepthTexture);

//	m_pEngine->GetDevice()->GenerateRenderTarget(m_pShadowDepthMapRenderTarget, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE);
//	m_pShadowDepthMapRenderTarget->Resize(Vector2(MSHADOW_TEXTURE_SIZE , MSHADOW_TEXTURE_SIZE));
}

void MForwardShadowMapWork::ReleaseRenderTargets()
{
// 	if (m_pShadowDepthMapRenderTarget)
// 	{
// 		m_pShadowDepthMapRenderTarget->DeleteLater();
// 		m_pShadowDepthMapRenderTarget = nullptr;
// 	}

// 	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
// 	{
// 		if (m_vShadowDepthTexture[i])
// 		{
// 			m_vShadowDepthTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
// 			delete m_vShadowDepthTexture[i];
// 			m_vShadowDepthTexture[i] = nullptr;
// 		}
// 	}
}

void MForwardShadowMapWork::InitializeShaderParamSet()
{
	m_pWorldMatrixParam = new MShaderConstantParam();
	m_pWorldMatrixParam->unSet = 1;
	m_pWorldMatrixParam->unBinding = 0;

	MStruct worldMatrixSrt;
	worldMatrixSrt.AppendMVariant("U_matCamProj", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matLightProj", Matrix4());

	m_pWorldMatrixParam->var = worldMatrixSrt;

	m_FrameParamSet.m_vParams.push_back(m_pWorldMatrixParam);
}

void MForwardShadowMapWork::ReleaseShaderParamSet()
{
	m_FrameParamSet.DestroyBuffer(GetEngine()->GetDevice());
}

void MForwardShadowMapWork::InitializeMaterial()
{
	m_pStaticMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_STATIC);
	m_pStaticMaterial->AddRef();

	m_pAnimMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_SKELETON);
	m_pAnimMaterial->AddRef();
}

void MForwardShadowMapWork::ReleaseMaterial()
{
	m_pStaticMaterial->SubRef();
	m_pStaticMaterial = nullptr;

	m_pAnimMaterial->SubRef();
	m_pAnimMaterial = nullptr;
}
