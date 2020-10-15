#include "MForwardShadowMapWork.h"
#include "MTransparentRenderTarget.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MScene.h"
#include "Model/MIMeshInstance.h"
#include "Model/MModelInstance.h"
#include "Light/MDirectionalLight.h"

#include "MSkeleton.h"
#include "MForwardRenderProgram.h"

M_OBJECT_IMPLEMENT(MForwardShadowMapWork, MObject)

MForwardShadowMapWork::MForwardShadowMapWork()
	: MObject()
	, m_pRenderProgram(nullptr)
	, m_pShadowDepthMapRenderTarget(nullptr)
	, m_vShadowDepthTexture()
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

	InitializeRenderTargets();
	InitializeShaderParamSet();
	InitializeMaterial();
	InitializeRenderPass();
}

void MForwardShadowMapWork::Release()
{
	ReleaseRenderPass();
	ReleaseMaterial();
	ReleaseShaderParamSet();
	ReleaseRenderTargets();
}

void MForwardShadowMapWork::InitializeRenderPass()
{
	m_ShadowRenderPass.m_DepthDesc.bClearWhenRender = true;

	GetEngine()->GetDevice()->GenerateRenderPass(&m_ShadowRenderPass, m_pShadowDepthMapRenderTarget);
}

void MForwardShadowMapWork::ReleaseRenderPass()
{
	GetEngine()->GetDevice()->DestroyRenderPass(&m_ShadowRenderPass);
}

void MForwardShadowMapWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MForwardShadowMapWork::DrawShadowMap(MForwardRenderProgram::MRenderInfo& info)
{
	info.pDirectionalLight = info.pScene->FindActiveDirectionLight();
	if (nullptr == info.pDirectionalLight)
		return;

	std::vector<MShadowRenderGroup> vShadowMeshGroup;

	UpdateRenderInfo(info, vShadowMeshGroup);
	
	if (vShadowMeshGroup.empty())
		return;

	info.pRenderer->BeginRenderPass(&m_ShadowRenderPass, m_pShadowDepthMapRenderTarget);

	RenderToShadowMap(info, vShadowMeshGroup);

	info.pRenderer->EndRenderPass();

}

void MForwardShadowMapWork::UpdateRenderInfo(MForwardRenderProgram::MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup)
{
	Vector3 v3LightDir = info.pDirectionalLight->GetWorldDirection();

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

			for (MNode* pChild : pModelIns->GetFixedChildren())
			{
				if (MIMeshInstance* pMeshIns = pChild->DynamicCast<MIMeshInstance>())
				{
					if (pMeshIns->GetVisible() && pMeshIns->GetGenerateDirLightShadow())
					{
						const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
						if (info.pViewport->GetCameraFrustum()->ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
						{
							group.vMeshInstances.push_back(pMeshIns);
							pBounds->UnionMinMax(v3ShadowMin, v3ShadowMax);
						}
					}
				}
			}
		}
	}

	info.cShadowRenderAABB.SetMinMax(v3ShadowMin, v3ShadowMax);

	info.m4DirLightInvProj = info.pViewport->GetLightInverseProjection(info.pDirectionalLight, info.cMeshRenderAABB, info.cShadowRenderAABB);

}

void MForwardShadowMapWork::RenderToShadowMap(MForwardRenderProgram::MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup)
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
	m_pShadowDepthMapRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MRenderDepthTexture* pDepthTexture = new MRenderDepthTexture();
		pDepthTexture->SetSize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
		pDepthTexture->GenerateBuffer(m_pEngine->GetDevice());

		m_vShadowDepthTexture[i] = pDepthTexture;
	}

	m_pShadowDepthMapRenderTarget->SetDepthTexture(m_vShadowDepthTexture);

//	m_pEngine->GetDevice()->GenerateRenderTarget(m_pShadowDepthMapRenderTarget, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE);
	m_pShadowDepthMapRenderTarget->Resize(Vector2(MSHADOW_TEXTURE_SIZE , MSHADOW_TEXTURE_SIZE));
}

void MForwardShadowMapWork::ReleaseRenderTargets()
{
	if (m_pShadowDepthMapRenderTarget)
	{
		m_pEngine->GetDevice()->DestroyRenderTarget(m_pShadowDepthMapRenderTarget);
		m_pShadowDepthMapRenderTarget->DeleteLater();
		m_pShadowDepthMapRenderTarget = nullptr;
	}

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (m_vShadowDepthTexture[i])
		{
			m_vShadowDepthTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			delete m_vShadowDepthTexture[i];
			m_vShadowDepthTexture[i] = nullptr;
		}
	}
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
	MMaterialResource* pShadowMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW);
	m_pStaticMaterial = pShadowMaterialRes;

	MMaterialResource* pShadowWithAnimMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_ANIM);
	m_pAnimMaterial = pShadowWithAnimMaterialRes;
}

void MForwardShadowMapWork::ReleaseMaterial()
{
}
