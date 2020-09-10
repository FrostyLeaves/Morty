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

#include "MForwardRenderProgram.h"

M_OBJECT_IMPLEMENT(MForwardShadowMapWork, MObject)

MForwardShadowMapWork::MForwardShadowMapWork()
	: MObject()
	, m_pRenderProgram(nullptr)
	, m_pShadowDepthMapRenderTarget(nullptr)
	, m_vShadowDepthTexture()
{
	
}

MForwardShadowMapWork::~MForwardShadowMapWork()
{
}

void MForwardShadowMapWork::DrawShadowMap(MForwardRenderProgram::MRenderInfo& info)
{
	info.pDirectionalLight = info.pScene->FindActiveDirectionLight();
	if (nullptr == info.pDirectionalLight)
		return;

	Vector3 v3LightDir = info.pDirectionalLight->GetWorldDirection();

	Vector3 v3ShadowMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3ShadowMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	info.vShadowGroup.push_back(MShadowRenderGroup());

	std::vector<MModelInstance*>& vModels = *info.pScene->GetAllModelInstance();
	for (MModelInstance* pModelIns : vModels)
	{
		if (pModelIns->GetVisibleRecursively() && pModelIns->GetGenerateDirLightShadow())
		{
			MShadowRenderGroup* pGroup = nullptr;
			if (pModelIns->GetSkeleton())
			{
				info.vShadowGroup.push_back(MShadowRenderGroup());
				pGroup = &info.vShadowGroup.back();
				pGroup->pSkeletonInstance = pModelIns->GetSkeleton();
			}
			else
			{
				pGroup = &info.vShadowGroup.front();
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

	if (nullptr == m_pShadowDepthMapRenderTarget || nullptr == info.pScene->FindActiveDirectionLight())
	{
		return;
	}


	//TODO 只有在需要ShadowMap时才进行渲染的优化
	m_pShadowDepthMapRenderTarget->Render(info.pRenderer, info.m4DirLightInvProj, &info.vShadowGroup);

	
}

void MForwardShadowMapWork::OnCreated()
{
	Super::OnCreated();

	InitializeRenderTargets();
}

void MForwardShadowMapWork::OnDelete()
{
	ReleaseRenderTargets();

	Super::OnDelete();
}

void MForwardShadowMapWork::InitializeRenderTargets()
{
	m_pShadowDepthMapRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MShadowTextureRenderTarget>();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MRenderDepthTexture* pDepthTexture = new MRenderDepthTexture();
		pDepthTexture->SetSize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
		pDepthTexture->GenerateBuffer(m_pEngine->GetDevice());

		m_vShadowDepthTexture[i] = pDepthTexture;
	}

	m_pShadowDepthMapRenderTarget->SetDepthTexture(m_vShadowDepthTexture);

	m_pEngine->GetDevice()->GenerateRenderTarget(m_pShadowDepthMapRenderTarget, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE);
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
