#include "MGPUCullingRenderWork.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Material/MComputeDispatcher.h"
#include "MergeInstancing/MMergeInstancingSubSystem.h"
#include "Render/MVertex.h"
#include "Utility/MBounds.h"


MORTY_CLASS_IMPLEMENT(MGPUCullingRenderWork, MObject)

MGPUCullingRenderWork::MGPUCullingRenderWork()
	: MObject()
{

}

MGPUCullingRenderWork::~MGPUCullingRenderWork()
{

}

void MGPUCullingRenderWork::OnCreated()
{
	Super::OnCreated();

	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();

	m_pCullingComputeDispatcher = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pCullingComputeDispatcher->LoadComputeShader("Shader/cull.mcs");


	m_cullingInstanceBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_cullingInstanceBuffer.m_eUsageType = MBuffer::MUsageType::EStorage;

	m_cullingIndirectDrawBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
	m_cullingIndirectDrawBuffer.m_eUsageType = MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect;

	//m_cullingIndirectDrawShadowBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
	//m_cullingIndirectDrawShadowBuffer.m_eUsageType = MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect;

	m_cullingDrawCallBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
	m_cullingDrawCallBuffer.m_eUsageType = MBuffer::MUsageType::EStorage;

#if MORTY_DEBUG
	m_cullingInstanceBuffer.m_strDebugBufferName = "Culling Instance Buffer";
	m_cullingIndirectDrawBuffer.m_strDebugBufferName = "Culling Indirect Draw Buffer";
	m_cullingDrawCallBuffer.m_strDebugBufferName = "Culling Draw Call";
	//m_cullingIndirectDrawShadowBuffer.m_strDebugBufferName = "Culling Indirect Draw Shadow Buffer";
#endif

}

void MGPUCullingRenderWork::OnDelete()
{
	Super::OnDelete();

	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	m_pCullingComputeDispatcher->DeleteLater();
	m_pCullingComputeDispatcher = nullptr;

	m_cullingInstanceBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_cullingIndirectDrawBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	//m_cullingIndirectDrawShadowBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_cullingDrawCallBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MGPUCullingRenderWork::CollectCullingGroup(MRenderInfo& info)
{
	ClearCullingGroup();


	MEngine* pEngine = GetEngine();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();
	MViewport* pViewport = info.pViewport;
	MScene* pScene = pViewport->GetScene();
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;


	MMergeInstancingSubSystem* pMergeInstancingSubSystem = pScene->GetSubSystem<MMergeInstancingSubSystem>();
	const MMergeInstancingMesh* pMergeInstancingMesh = pMergeInstancingSubSystem->GetMergeInstancingMesh();

	if (!pMergeInstancingSubSystem || !pMergeInstancingMesh)
	{
		return;
	}


	auto&& tBatchInstanceTable = pMergeInstancingSubSystem->GetMaterialToBatchInstanceTable();




	int nInstanceCount = 0;
	const MBuffer* pVertexBuffer = pMergeInstancingMesh->GetVertexBuffer();
	const MBuffer* pIndexBuffer = pMergeInstancingMesh->GetIndexBuffer();
	MMaterialCullingGroup* pMaterialCullingGroup = nullptr;
	MVariantArray* pTransformArray = nullptr;
	MVariantArray* pClusterArray = nullptr;

	auto createNewGroupFunc = [&](const std::shared_ptr<MMaterial>& pMaterial, MMaterialBatchGroup* pMaterialBatchGroup)
	{
		if (pMaterialCullingGroup)
		{
			nInstanceCount += pMaterialCullingGroup->nClusterCount;
		}
		m_vCullingInstanceGroup.push_back({});
		pMaterialCullingGroup = &m_vCullingInstanceGroup.back();
		pMaterialCullingGroup->pMaterial = pMaterial;
		pMaterialCullingGroup->nClusterBeginIdx = m_vInstanceCullData.size();
		pMaterialCullingGroup->nClusterCount = 0;
		pMaterialCullingGroup->nTransformCount = 0;
		pMaterialCullingGroup->pVertexBuffer = pVertexBuffer;
		pMaterialCullingGroup->pIndexBuffer = pIndexBuffer;
		pMaterialCullingGroup->pMeshTransformProperty = pMaterialBatchGroup->pMaterial->GetMeshParamSet()->Clone();
		MORTY_ASSERT(pMaterialCullingGroup->pMeshTransformProperty);

		std::shared_ptr<MShaderConstantParam> pTransformParam = pMaterialCullingGroup->pMeshTransformProperty->FindConstantParam("_M_E_cbMeshMatrix");
		MORTY_ASSERT(pTransformParam);
		pTransformParam->SetDirty();

		pTransformArray = &pTransformParam->var.GetValue<MVariantStruct>().GetVariant<MVariantArray>("u_meshMatrix");
		MORTY_ASSERT(pTransformArray);

		pClusterArray = &pTransformParam->var.GetValue<MVariantStruct>().GetVariant<MVariantArray>("u_meshClusterIndex");
		MORTY_ASSERT(pClusterArray);

		int* pInstanceBeginIndex = &pTransformParam->var.GetValue<MVariantStruct>().GetVariant<int>("u_meshInstanceBeginIndex");
		MORTY_ASSERT(pInstanceBeginIndex);
		(*pInstanceBeginIndex) = nInstanceCount;
	};

	for (auto&& pr : tBatchInstanceTable)
	{
		MMaterialBatchGroup* pMaterialBatchGroup = pr.second;
		if (!pMaterialBatchGroup)
		{
			continue;
		}

		createNewGroupFunc(pr.first, pMaterialBatchGroup);

		for (MRenderableMeshComponent* pMeshComponent : pMaterialBatchGroup->vMeshComponent)
		{
			MSceneComponent* pSceneComponent = pMeshComponent->GetEntity()->GetComponent<MSceneComponent>();
			if (!pSceneComponent)
			{
				continue;
			}

			Matrix4 matWorld = pSceneComponent->GetWorldTransform();
			Matrix3 matNormal = Matrix3(matWorld, 3, 3);

			if (pMaterialCullingGroup->nTransformCount >= pTransformArray->MemberNum())
			{
				createNewGroupFunc(pr.first, pMaterialBatchGroup);
			}

			(*pTransformArray)[pMaterialCullingGroup->nTransformCount].GetValue<MVariantStruct>().SetVariant("matWorld", matWorld);
			(*pTransformArray)[pMaterialCullingGroup->nTransformCount].GetValue<MVariantStruct>().SetVariant("matNormal", matNormal);
			++pMaterialCullingGroup->nTransformCount;

			const std::vector<MMergeInstancingMesh::MClusterData>& vMeshClusterGroup = pMergeInstancingSubSystem->GetMeshClusterGroup(pMeshComponent->GetMesh());

			for (const MMergeInstancingMesh::MClusterData& clusterData : vMeshClusterGroup)
			{
				const Vector3 scale = pSceneComponent->GetWorldScale();
				const float regularScale = (std::max)((std::max)(scale.x, scale.y), scale.z);
				MMergeInstanceCullData cullData;
				cullData.position = pSceneComponent->GetWorldPosition() + clusterData.boundsShpere.m_v3CenterPoint * regularScale;
				cullData.radius = clusterData.boundsShpere.m_fRadius * regularScale;

				for (size_t nLodIdx = 0; nLodIdx < MRenderGlobal::MESH_LOD_LEVEL_RANGE; ++nLodIdx)
				{
					//TODO LOD
					cullData.lods[nLodIdx].distance = 500 * (1 + nLodIdx) / MRenderGlobal::MESH_LOD_LEVEL_RANGE;
					cullData.lods[nLodIdx].firstIndex = clusterData.memoryInfo.begin / sizeof(uint32_t);
					cullData.lods[nLodIdx].indexCount = clusterData.memoryInfo.size / sizeof(uint32_t);
				}

				if (pMaterialCullingGroup->nClusterCount >= pClusterArray->MemberNum())
				{
					createNewGroupFunc(pr.first, pMaterialBatchGroup);
					(*pTransformArray)[pMaterialCullingGroup->nTransformCount].GetValue<MVariantStruct>().SetVariant("matWorld", matWorld);
					(*pTransformArray)[pMaterialCullingGroup->nTransformCount].GetValue<MVariantStruct>().SetVariant("matNormal", matNormal);
					++pMaterialCullingGroup->nTransformCount;
				}

				(*pClusterArray)[pMaterialCullingGroup->nClusterCount].SetValue(static_cast<int>(pMaterialCullingGroup->nTransformCount - 1));
				++pMaterialCullingGroup->nClusterCount;
				m_vInstanceCullData.push_back(cullData);
			}
		}
	}

	if (m_vInstanceCullData.empty())
	{
		return;
	}

	const size_t unCullingBufferSize = m_vInstanceCullData.size() * sizeof(MMergeInstanceCullData);
	if (m_cullingInstanceBuffer.GetSize() < unCullingBufferSize)
	{
		m_cullingInstanceBuffer.ReallocMemory(unCullingBufferSize);
		m_cullingInstanceBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_cullingInstanceBuffer.GenerateBuffer(pRenderSystem->GetDevice(), reinterpret_cast<MByte*>(m_vInstanceCullData.data()), unCullingBufferSize);
	}
	else
	{
		m_cullingInstanceBuffer.UploadBuffer(pRenderSystem->GetDevice(), reinterpret_cast<MByte*>(m_vInstanceCullData.data()), unCullingBufferSize);
	}

	const size_t unDrawBufferSize = m_vInstanceCullData.size() * sizeof(MDrawIndexedIndirectData);
	if (m_cullingIndirectDrawBuffer.GetSize() < unDrawBufferSize)
	{
		m_cullingIndirectDrawBuffer.ReallocMemory(unDrawBufferSize);
		m_cullingIndirectDrawBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_cullingIndirectDrawBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, unDrawBufferSize);
	}

	/*
	if (m_cullingIndirectDrawShadowBuffer.GetSize() < unDrawBufferSize)
	{
		m_cullingIndirectDrawShadowBuffer.ReallocMemory(unDrawBufferSize);
		m_cullingIndirectDrawShadowBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_cullingIndirectDrawShadowBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, unDrawBufferSize);
	}
	*/

	const size_t unDrawCallBufferSize = m_vInstanceCullData.size() * sizeof(MMergeInstanceDrawCallOutput);
	if (m_cullingDrawCallBuffer.GetSize() < unDrawCallBufferSize)
	{
		m_cullingDrawCallBuffer.ReallocMemory(unDrawCallBufferSize);
		m_cullingDrawCallBuffer.DestroyBuffer(pRenderSystem->GetDevice());
		m_cullingDrawCallBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, unDrawCallBufferSize);
	}


	const std::shared_ptr<MShaderPropertyBlock>& params = m_pCullingComputeDispatcher->GetShaderParamSets()[0];

	if (std::shared_ptr<MShaderStorageParam>&& pStorageParam = params->FindStorageParam("instances"))
	{
		pStorageParam->pBuffer = &m_cullingInstanceBuffer;
		pStorageParam->SetDirty();
	}

	if (std::shared_ptr<MShaderStorageParam>&& pStorageParam = params->FindStorageParam("indirectDraws"))
	{
		pStorageParam->pBuffer = &m_cullingIndirectDrawBuffer;
		pStorageParam->SetDirty();
	}

	/*
	if (std::shared_ptr<MShaderStorageParam>&& pStorageParam = params->FindStorageParam("indirectShadowDraws"))
	{
		pStorageParam->pBuffer = &m_cullingIndirectDrawShadowBuffer;
		pStorageParam->SetDirty();
	}
	*/

	if (std::shared_ptr<MShaderStorageParam>&& pStorageParam = params->FindStorageParam("uboOut"))
	{
		pStorageParam->pBuffer = &m_cullingDrawCallBuffer;
		pStorageParam->SetDirty();
	}

}

void MGPUCullingRenderWork::UpdateCameraFrustum(MRenderInfo& info)
{
	MViewport* pViewport = info.pViewport;

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pCullingComputeDispatcher->GetShaderParamSets()[0];

	if (std::shared_ptr<MShaderConstantParam>&& pConstantParam = params->FindConstantParam("ubo"))
	{
		MVariantStruct& sut = pConstantParam->var.GetValue<MVariantStruct>();
		{
			MORTY_ASSERT(info.pCameraEntity);
			MSceneComponent* pCameraSceneComponent = info.pCameraEntity->GetComponent<MSceneComponent>();
			MORTY_ASSERT(pCameraSceneComponent);
			sut.SetVariant("cameraPos", Vector4(pCameraSceneComponent->GetWorldPosition(), 1.0f));
			MVariantArray& cFrustumArray = sut.GetVariant<MVariantArray>("frustumPlanes");
			{
				MCameraFrustum& cameraFrustum = pViewport->GetCameraFrustum();
				for (size_t planeIdx = 0; planeIdx < 6; ++planeIdx)
				{
					const Vector4& plane = cameraFrustum.GetPlane(planeIdx).m_v4Plane;
					cFrustumArray[planeIdx].SetValue(plane / Vector3(plane.x, plane.y, plane.z).Length());
				}
			}
		}

		pConstantParam->SetDirty();
	}
}

void MGPUCullingRenderWork::DispatchCullingJob(MRenderInfo& info)
{
	if (m_cullingIndirectDrawBuffer.GetSize() == 0)
	{
		return;
	}

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	pCommand->AddGraphToComputeBarrier({ &m_cullingIndirectDrawBuffer, /*&m_cullingIndirectDrawShadowBuffer*/ });

	pCommand->DispatchComputeJob(m_pCullingComputeDispatcher, m_vInstanceCullData.size() / 16 + (m_vInstanceCullData.size() % 16 ? 1 : 0), 1, 1);

	pCommand->AddComputeToGraphBarrier({ &m_cullingIndirectDrawBuffer, /*&m_cullingIndirectDrawShadowBuffer*/ });
}

void MGPUCullingRenderWork::ClearCullingGroup()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	for (MMaterialCullingGroup& group : m_vCullingInstanceGroup)
	{
		group.pMeshTransformProperty->DestroyBuffer(pRenderSystem->GetDevice());
		group.pMeshTransformProperty = nullptr;
	}

	m_vCullingInstanceGroup.clear();
	m_vInstanceCullData.clear();
}
