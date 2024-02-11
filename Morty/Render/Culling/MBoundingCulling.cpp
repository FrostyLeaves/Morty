#include "MBoundingCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Batch/MMaterialBatchGroup.h"
#include "Render/MRenderCommand.h"

using namespace morty;

void MBoundingCulling::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_drawIndirectBuffer = MBuffer::CreateHostVisibleIndirectBuffer("MBounding Culling Draw Instance Buffer");
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

void MBoundingCulling::UploadBuffer(MIRenderCommand* pCommand)
{
	const size_t nDrawIndirectBufferSize = m_vDrawIndirectData.size() * sizeof(MDrawIndexedIndirectData);
	pCommand->UploadBuffer(&m_drawIndirectBuffer, reinterpret_cast<MByte*>(m_vDrawIndirectData.data()), nDrawIndirectBufferSize);
}

void MBoundingCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
	std::vector<MDrawIndexedIndirectData>& vDrawIndirectData = m_vDrawIndirectData;

	vDrawIndirectData.clear();
	m_vCullingInstanceGroup.clear();

	auto createNewGroupFunc = [&](const std::shared_ptr<MMaterial>& pMaterial, MInstanceBatchGroup* pInstanceBatchGroup)
	{
		const auto pMeshProperty = pInstanceBatchGroup->GetMeshProperty();
		pMeshProperty->SetValue(MShaderPropertyName::MESH_INSTANCE_BEGIN_INDEX, 0);

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
						static_cast<uint32_t>(data.indexInfo.size),
						1,
                        static_cast<uint32_t>(data.indexInfo.begin),
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
}
