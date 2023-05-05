#include "MSceneGPUCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MergeInstancing/MRenderableMeshGroup.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Material/MComputeDispatcher.h"
#include "System/MObjectSystem.h"
#include "Render/MRenderCommand.h"

void MSceneGPUCulling::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_cullingInputBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_cullingInputBuffer.m_eUsageType = MBuffer::MUsageType::EStorage;

	m_drawIndirectBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
	m_drawIndirectBuffer.m_eUsageType = MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect;

	m_cullingOutputBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_cullingOutputBuffer.m_eUsageType = MBuffer::MUsageType::EStorage;

	constexpr size_t nOutputDataSize = sizeof(MMergeInstanceDrawCallOutput);
	m_cullingOutputBuffer.ReallocMemory(nOutputDataSize);
	m_cullingOutputBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nOutputDataSize);



#if MORTY_DEBUG
	m_cullingInputBuffer.m_strDebugBufferName = "Culling Input Buffer";
	m_drawIndirectBuffer.m_strDebugBufferName = "GPU Culling Draw Instance Buffer";
	m_cullingOutputBuffer.m_strDebugBufferName = "Culling Output Buffer";
#endif


	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	m_pCullingComputeDispatcher = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pCullingComputeDispatcher->LoadComputeShader("Shader/cull.mcs");


	const std::shared_ptr<MShaderPropertyBlock>& params = m_pCullingComputeDispatcher->GetShaderParamSets()[0];

	if (std::shared_ptr<MShaderStorageParam> pStorageParam = params->FindStorageParam("instances"))
	{
		pStorageParam->pBuffer = &m_cullingInputBuffer;
		pStorageParam->SetDirty();
	}

	if (std::shared_ptr<MShaderStorageParam> pStorageParam = params->FindStorageParam("indirectDraws"))
	{
		pStorageParam->pBuffer = &m_drawIndirectBuffer;
		pStorageParam->SetDirty();
	}

	if (std::shared_ptr<MShaderStorageParam>&& pStorageParam = params->FindStorageParam("uboOut"))
	{
		pStorageParam->pBuffer = &m_cullingOutputBuffer;
		pStorageParam->SetDirty();
	}
}

void MSceneGPUCulling::Release()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_cullingInputBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_cullingOutputBuffer.DestroyBuffer(pRenderSystem->GetDevice());

	m_pCullingComputeDispatcher->DeleteLater();
	m_pCullingComputeDispatcher = nullptr;
}

void MSceneGPUCulling::AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter)
{
	m_vFilter.push_back(pFilter);
}

void MSceneGPUCulling::UpdateCullingCamera()
{
	const std::shared_ptr<MShaderPropertyBlock>& params = m_pCullingComputeDispatcher->GetShaderParamSets()[0];

	std::shared_ptr<MShaderConstantParam> pConstantParam = params->FindConstantParam("ubo");
	if(!pConstantParam)
	{
		MORTY_ASSERT(pConstantParam);
		return;
	}
	
	MVariantStruct& sut = pConstantParam->var.GetValue<MVariantStruct>();
	sut.SetVariant("cameraPos", Vector4(m_v3CameraPosition, 1.0f));
	MVariantArray& cFrustumArray = sut.GetVariant<MVariantArray>("frustumPlanes");
	{
		for (size_t planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const Vector4& plane = m_cameraFrustum.GetPlane(planeIdx).m_v4Plane;
			cFrustumArray[planeIdx].SetValue(plane / Vector3(plane.x, plane.y, plane.z).Length());
		}
	}

	pConstantParam->SetDirty();
}

void MSceneGPUCulling::Culling(const std::vector<MRenderableMaterialGroup*>& vInstanceGroup)
{
	if (!m_pCommand)
	{
		MORTY_ASSERT(m_pCommand);
		return;
	}

	UpdateCullingCamera();
	m_vCullingInstanceGroup.clear();


	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::vector<MMergeInstanceCullData> vInstanceCullingData;

	auto createNewGroupFunc = [&](const std::shared_ptr<MMaterial>& pMaterial, MInstanceBatchGroup* pInstanceBatchGroup)
	{
		int nIndirectBeginIdx = vInstanceCullingData.size();
		const auto pMeshProperty = pInstanceBatchGroup->GetMeshProperty();
		pMeshProperty->SetValue("u_meshInstanceBeginIndex", nIndirectBeginIdx);

		m_vCullingInstanceGroup.push_back({});
		MMaterialCullingGroup* pMaterialCullingGroup = &m_vCullingInstanceGroup.back();
		pMaterialCullingGroup->pMaterial = pMaterial;
		pMaterialCullingGroup->nIndirectBeginIdx = vInstanceCullingData.size();
		pMaterialCullingGroup->pMeshTransformProperty = pMeshProperty;
	};

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	for (MRenderableMaterialGroup* pMaterialGroup : vInstanceGroup)
	{
		if (pMaterialGroup->GetMaterial() == nullptr)
		{
			MORTY_ASSERT(pMaterialGroup->GetMaterial());
			continue;
		}

		for (MInstanceBatchGroup* pInstanceGroup : pMaterialGroup->GetInstanceBatchGroup())
		{
			createNewGroupFunc(pMaterialGroup->GetMaterial(), pInstanceGroup);

			pInstanceGroup->InstanceExecute([&](const MRenderableMeshInstance& instance, size_t nIdx)
			{
				for(const auto& pFilter : m_vFilter)
				{
				    if (!pFilter->Filter(&instance))
				    {
						return;
				    }
				}
	
				const MMeshManager::MMeshData& data = pMeshManager->FindMesh(instance.pMesh);

				MMergeInstanceCullData cullData;
				cullData.position = instance.boundsWithTransform.m_v3CenterPoint;
				cullData.radius = instance.boundsWithTransform.m_v3HalfLength.Length();
				for (size_t nLodIdx = 0; nLodIdx < MRenderGlobal::MESH_LOD_LEVEL_RANGE; ++nLodIdx)
				{
					//TODO LOD
					cullData.lods[nLodIdx].distance = 500 * (1 + nLodIdx) / MRenderGlobal::MESH_LOD_LEVEL_RANGE;
					cullData.lods[nLodIdx].firstIndex = data.indexInfo.begin;
					cullData.lods[nLodIdx].indexCount = data.indexInfo.size;
				}
				vInstanceCullingData.push_back(cullData);
			});

			if (!m_vCullingInstanceGroup.empty())
			{
				m_vCullingInstanceGroup.back().nIndirectCount = vInstanceCullingData.size() - m_vCullingInstanceGroup.back().nIndirectBeginIdx;
			}
		}
	}

	if (vInstanceCullingData.empty())
	{
		m_vCullingInstanceGroup.clear();
		return;
	}

	const size_t unCullingBufferSize = vInstanceCullingData.size() * sizeof(MMergeInstanceCullData);
	if (m_cullingInputBuffer.GetSize() < unCullingBufferSize)
	{
		m_cullingInputBuffer.ReallocMemory(unCullingBufferSize);
		m_cullingInputBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_cullingInputBuffer.GenerateBuffer(pRenderSystem->GetDevice(), reinterpret_cast<MByte*>(vInstanceCullingData.data()), unCullingBufferSize);
	}
	else
	{
		m_cullingInputBuffer.UploadBuffer(pRenderSystem->GetDevice(), reinterpret_cast<MByte*>(vInstanceCullingData.data()), unCullingBufferSize);
	}

	const size_t nDrawIndirectBufferSize = vInstanceCullingData.size() * sizeof(MDrawIndexedIndirectData);
	if (m_drawIndirectBuffer.GetSize() < nDrawIndirectBufferSize)
	{
		m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
		m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);
	}

	

	m_pCommand->AddGraphToComputeBarrier({ &m_drawIndirectBuffer });

	m_pCommand->DispatchComputeJob(m_pCullingComputeDispatcher, vInstanceCullingData.size() / 16 + (vInstanceCullingData.size() % 16 ? 1 : 0), 1, 1);

	m_pCommand->AddComputeToGraphBarrier({ &m_drawIndirectBuffer });
}
