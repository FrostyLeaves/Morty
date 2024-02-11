#include "MCascadedShadowCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Basic/MViewport.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Batch/MMaterialBatchGroup.h"

#include "Batch/BatchGroup/MInstanceBatchGroup.h"
#include "Render/MVertex.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

using namespace morty;

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

void MCascadedShadowCulling::SetCamera(MEntity* pCameraEntity)
{
	m_pCameraEntity = pCameraEntity;
}

void MCascadedShadowCulling::SetDirectionalLight(MEntity* pDirectionalLight)
{
	m_pDirectionalLight = pDirectionalLight;
}

void MCascadedShadowCulling::UploadBuffer(MIRenderCommand* pCommand)
{
	const size_t nDrawIndirectBufferSize = m_vDrawIndirectData.size() * sizeof(MDrawIndexedIndirectData);
	pCommand->UploadBuffer(&m_drawIndirectBuffer, reinterpret_cast<MByte*>(m_vDrawIndirectData.data()), nDrawIndirectBufferSize);
}

void MCascadedShadowCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
	auto vCascadedSplitData = MShadowMapUtil::CascadedSplitCameraFrustum(m_pViewport);

#if MORTY_VXGI_ENABLE
	// shadow map must contain the GI region.
	const auto vCascadedPsrBounds = MShadowMapUtil::GetVoxelMapBounds(m_pViewport, vCascadedSplitData);
	const auto vCascadedFilter = MShadowMapUtil::GetBoundsCullingFilter(m_pViewport, vCascadedPsrBounds);
#else
	const auto vCascadedPsrBounds = MShadowMapUtil::GetCameraFrustumBounds(m_pViewport, vCascadedSplitData);
	const auto vCascadedFilter = MShadowMapUtil::GetCameraFrustumCullingFilter(m_pViewport, vCascadedSplitData);
#endif

	CullingForDrawInstancing(vInstanceGroup, vCascadedFilter);
	m_vCascadedRenderData = MShadowMapUtil::CalculateRenderData(m_pViewport, m_pCameraEntity, vCascadedSplitData, vCascadedPsrBounds, m_vCascadedPscBounds);
}

void MCascadedShadowCulling::CullingForDrawInstancing(const std::vector<MMaterialBatchGroup*>& vInstanceGroup, const MCascadedArray<std::unique_ptr<IRenderableFilter>>& vCascadedFilter)
{
	if (!m_pDirectionalLight)
	{
		return;
	}

	std::vector<MDrawIndexedIndirectData>& vDrawIndirectData = m_vDrawIndirectData;

	vDrawIndirectData.clear();
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

	MCascadedArray<bool> vShadowBoundsValid;
	MCascadedArray<Vector3> vShadowBoundsMin;
	MCascadedArray<Vector3> vShadowBoundsMax;
	vShadowBoundsValid.fill(false);
	vShadowBoundsMin.fill(Vector3(+FLT_MAX, +FLT_MAX, +FLT_MAX));
	vShadowBoundsMax.fill(Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX));

	for (const MMaterialBatchGroup* pMaterialGroup : vInstanceGroup)
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

			pInstanceGroup->InstanceExecute([&](const MMeshInstanceRenderProxy& instance, size_t nIdx) {
					const MBoundsAABB& bounds = instance.boundsWithTransform;

			        size_t nOutsideCount = 0;
			        for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			        {
						if (!vCascadedFilter[nCascadedIdx]->Filter(&instance))
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

}
