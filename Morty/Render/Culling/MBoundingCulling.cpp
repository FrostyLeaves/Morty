#include "MBoundingCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Batch/MMaterialBatchGroup.h"


void MBoundingCulling::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_drawIndirectBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_drawIndirectBuffer.m_eUsageType = MBuffer::MUsageType::EIndirect;

#if MORTY_DEBUG
	m_drawIndirectBuffer.m_strDebugBufferName = "MBounding Culling Draw Instance Buffer";
#endif
}

void MBoundingCulling::Release()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MBoundingCulling::AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter)
{
	m_vFilter.push_back(pFilter);
}

void MBoundingCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::vector<MDrawIndexedIndirectData> vDrawIndirectData;
	m_vCullingInstanceGroup.clear();

	auto createNewGroupFunc = [&](const std::shared_ptr<MMaterial>& pMaterial, MInstanceBatchGroup* pInstanceBatchGroup)
	{
		const auto pMeshProperty = pInstanceBatchGroup->GetMeshProperty();
		pMeshProperty->SetValue("u_meshInstanceBeginIndex", 0);

		m_vCullingInstanceGroup.push_back({});
		MMaterialCullingGroup* pMaterialCullingGroup = &m_vCullingInstanceGroup.back();
		pMaterialCullingGroup->pMaterial = pMaterial;
		pMaterialCullingGroup->nIndirectBeginIdx = vDrawIndirectData.size();
		pMaterialCullingGroup->pMeshTransformProperty = pMeshProperty;
	};

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	for (MMaterialBatchGroup* pMaterialGroup : vInstanceGroup)
	{
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
					for (const auto& pFilter : m_vFilter)
					{
						if (!pFilter->Filter(&instance))
						{
							return;
						}
					}

					const MMeshManager::MMeshData& data = pMeshManager->FindMesh(instance.pMesh);
					const MDrawIndexedIndirectData indirectData = {
						data.indexInfo.size,
						1,
						data.indexInfo.begin,
						0,
						static_cast<uint32_t>(nIdx)
					};
					vDrawIndirectData.push_back(indirectData);
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

}