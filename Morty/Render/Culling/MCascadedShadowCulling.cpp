#include "MCascadedShadowCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Batch/MMaterialBatchGroup.h"


void MCascadedShadowCulling::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_drawIndirectBuffer = MBuffer::CreateHostVisibleIndirectBuffer("Camera Culling Draw Instance Buffer");
}

void MCascadedShadowCulling::Release()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MCascadedShadowCulling::SetViewport(MViewport* pViewport)
{
	m_pViewport = pViewport;
}

void MCascadedShadowCulling::SetCamera(MEntity* pCameraEntity)
{
	m_pCameraEntity = pCameraEntity;
}

void MCascadedShadowCulling::SetDirectionalLight(MEntity* pDirectionalLight)
{
	m_pDirectionalLight = pDirectionalLight;
}

void MCascadedShadowCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
	if (!m_pDirectionalLight)
	{
		return;
	}

	auto pLightSceneComponent = m_pDirectionalLight->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
	{
		MORTY_ASSERT(pLightSceneComponent);
		return;
	}

	Vector3 v3LightDirection = pLightSceneComponent->GetForward();

	auto vCascadedData = MShadowMapUtil::CascadedSplitCameraFrustum(m_pViewport);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::vector<MDrawIndexedIndirectData> vDrawIndirectData;
	m_vCullingInstanceGroup.clear();

	auto createNewGroupFunc = [&](const std::shared_ptr<MMaterial>& pMaterial, MInstanceBatchGroup* pInstanceBatchGroup)
	{
		//int nIndirectBeginIdx = vDrawIndirectData.size();
		const auto pMeshProperty = pInstanceBatchGroup->GetMeshProperty();
		pMeshProperty->SetValue(MShaderPropertyName::MESH_INSTANCE_BEGIN_INDEX, 0);

		m_vCullingInstanceGroup.push_back({});
		MMaterialCullingGroup* pMaterialCullingGroup = &m_vCullingInstanceGroup.back();
		pMaterialCullingGroup->pMaterial = pMaterial;
		pMaterialCullingGroup->nIndirectBeginIdx = vDrawIndirectData.size();
		pMaterialCullingGroup->pMeshTransformProperty = pMeshProperty;
	};

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	std::array<bool, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vShadowBoundsValid;
	std::array<Vector3, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vShadowBoundsMin;
	std::array<Vector3, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vShadowBoundsMax;
	vShadowBoundsValid.fill(false);
	vShadowBoundsMin.fill(Vector3(+FLT_MAX, +FLT_MAX, +FLT_MAX));
	vShadowBoundsMax.fill(Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX));

	for (MMaterialBatchGroup* pMaterialGroup : vInstanceGroup)
	{
		if (!pMaterialGroup)
		{
			continue;
		}

		if (pMaterialGroup->GetMaterial() == nullptr)
		{
			MORTY_ASSERT(pMaterialGroup->GetMaterial());
			continue;
		}

		for (MInstanceBatchGroup* pInstanceGroup : pMaterialGroup->GetInstanceBatchGroup())
		{
			createNewGroupFunc(pMaterialGroup->GetMaterial(), pInstanceGroup);

			pInstanceGroup->InstanceExecute([&](const MMeshInstanceRenderProxy& instance, size_t nIdx)
			{
				const MBoundsAABB& bounds = instance.boundsWithTransform;

				size_t nOutsideCount = 0;
			    for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			    {
				    if (MCameraFrustum::EOUTSIDE == vCascadedData[nCascadedIdx].cCameraFrustum.ContainTest(bounds, v3LightDirection))
				    {
						++nOutsideCount;
					    continue;
				    }

					vShadowBoundsValid[nCascadedIdx] = true;
					bounds.UnionMinMax(vShadowBoundsMin[nCascadedIdx], vShadowBoundsMax[nCascadedIdx]);
			    }

				if (nOutsideCount < MRenderGlobal::CASCADED_SHADOW_MAP_NUM)
				{
					const MMeshManager::MMeshData& data = pMeshManager->FindMesh(instance.pMesh);
					const MDrawIndexedIndirectData indirectData = {
                        static_cast<uint32_t>(data.indexInfo.size),
							1,
                        static_cast<uint32_t>(data.indexInfo.begin),
							0,
							static_cast<uint32_t>(nIdx)
					};
					vDrawIndirectData.push_back(indirectData);
				}
			});

			if (!m_vCullingInstanceGroup.empty())
			{
				m_vCullingInstanceGroup.back().nIndirectCount = vDrawIndirectData.size() - m_vCullingInstanceGroup.back().nIndirectBeginIdx;
			}
		}

	}


	const size_t nDrawIndirectBufferSize = vDrawIndirectData.size() * sizeof(MDrawIndexedIndirectData);
	if (m_drawIndirectBuffer.GetSize() < nDrawIndirectBufferSize)
	{
		m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
		m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), reinterpret_cast<MByte*>(vDrawIndirectData.data()), nDrawIndirectBufferSize);
	}
	else if (nDrawIndirectBufferSize > 0)
	{
		m_drawIndirectBuffer.UploadBuffer(pRenderSystem->GetDevice(), reinterpret_cast<MByte*>(vDrawIndirectData.data()), nDrawIndirectBufferSize);
	}

	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		if (vShadowBoundsValid[nCascadedIdx])
		{
			m_vCascadedPscBounds[nCascadedIdx].SetMinMax(vShadowBoundsMin[nCascadedIdx], vShadowBoundsMax[nCascadedIdx]);
		}
		else
		{
			m_vCascadedPscBounds[nCascadedIdx].SetMinMax(Vector3::Zero, Vector3::Zero);
		}
	}

	m_vCascadedRenderData = MShadowMapUtil::CalculateRenderData(m_pViewport, m_pCameraEntity, vCascadedData, m_vCascadedPscBounds);
}
