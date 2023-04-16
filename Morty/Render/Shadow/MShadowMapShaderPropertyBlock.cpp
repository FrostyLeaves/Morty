#include "MShadowMapShaderPropertyBlock.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Shadow/MShadowMapManager.h"
#include "Resource/MMaterialResource.h"

#include "Material/MMaterial.h"
#include "Material/MComputeDispatcher.h"
#include "Render/MVertex.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"

void MShadowMapShaderPropertyBlock::Initialize(MEngine* pEngine)
{
	MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/shadowmap.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/shadowmap.mps");
	m_pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	m_pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, "true");
	m_pMaterial->LoadVertexShader(vs);
	m_pMaterial->LoadPixelShader(ps);

	BindMaterial(m_pMaterial);

	m_pCullingDispatcher = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pCullingDispatcher->LoadComputeShader("Shader/Shadow/GpuDriven.mcs");

	m_drawIndirectBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
	m_drawIndirectBuffer.m_eUsageType = MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect;

	m_outputBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_outputBuffer.m_eUsageType = MBuffer::MUsageType::EStorage;

#if MORTY_DEBUG
	m_drawIndirectBuffer.m_strDebugBufferName = "Shadow Draw Indirect Buffer";
	m_outputBuffer.m_strDebugBufferName = "Shadow Output Buffer";
#endif
}

void MShadowMapShaderPropertyBlock::Release(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pMeshInstancePropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pMeshInstancePropertyBlock = nullptr;
	m_pMaterial = nullptr;

	m_pCullingDispatcher->DeleteLater();
	m_pCullingDispatcher = nullptr;

	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_outputBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MShadowMapShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MORTY_ASSERT(m_pShaderPropertyBlock = pMaterial->GetFrameParamSet()->Clone());
	MORTY_ASSERT(m_pMeshInstancePropertyBlock = pMaterial->GetMeshParamSet()->Clone());

	MORTY_ASSERT(m_pWorldMatrixParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneMatrix"));
}

void MShadowMapShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info) 
{
	MViewport* pViewport = info.pViewport;
	MORTY_ASSERT(pViewport);
	MEntity* pCameraEntity = info.pCameraEntity;
	MORTY_ASSERT(pCameraEntity);
	MScene* pScene = pViewport->GetScene();
	const MRenderSystem* pRenderSystem = pScene->GetEngine()->FindSystem<MRenderSystem>();
	auto* pShadowMapManager = pScene->GetManager<MShadowMapManager>();
	MORTY_ASSERT(pShadowMapManager);

	if (m_pWorldMatrixParam)
	{
		MVariantStruct& cStruct = m_pWorldMatrixParam->var.GetValue<MVariantStruct>();

		MVariantArray& cCamProjArray = cStruct.GetVariant<MVariantArray>("u_matCamProj");


		for (size_t nCascadedIdx = 0; nCascadedIdx < info.shadowRenderInfo.size(); ++nCascadedIdx)
		{
			cCamProjArray[nCascadedIdx].SetValue(info.shadowRenderInfo[nCascadedIdx].m4DirLightInvProj);
		}

		m_pWorldMatrixParam->SetDirty();
	}

	if (auto pMeshInstance = m_pMeshInstancePropertyBlock->FindStorageParam("u_meshMatrix"))
	{
		MBuffer* pBuffer = pShadowMapManager->GetTransformBuffer();
		if (pMeshInstance->pBuffer != pBuffer)
		{
			pMeshInstance->pBuffer = pBuffer;
			pMeshInstance->SetDirty();
		}
	}

	auto params = m_pCullingDispatcher->GetShaderParamSets()[0];
	const size_t nInstanceNum = pShadowMapManager->GetInstanceNum();
	m_nInstanceNum = nInstanceNum;

	if (std::shared_ptr<MShaderStorageParam> pStorageParam = params->FindStorageParam("instances"))
	{
		MBuffer* pBuffer = pShadowMapManager->GetInstanceBuffer();
		if (pStorageParam->pBuffer != pBuffer)
		{
			pStorageParam->pBuffer = pBuffer;
			pStorageParam->SetDirty();
		}
	}

	if (std::shared_ptr<MShaderStorageParam> pStorageParam = params->FindStorageParam("indirectDraws"))
	{
		const size_t nDrawIndirectBufferSize = nInstanceNum * sizeof(MDrawIndexedIndirectData);
		if (m_drawIndirectBuffer.GetSize() < nDrawIndirectBufferSize)
		{
			m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
			m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
			m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);
		}
		if (pStorageParam->pBuffer != &m_drawIndirectBuffer)
		{
			pStorageParam->pBuffer = &m_drawIndirectBuffer;
			pStorageParam->SetDirty();
		}
	}

	if (std::shared_ptr<MShaderStorageParam>&& pStorageParam = params->FindStorageParam("uboOut"))
	{
		const size_t nOutputBufferSize = nInstanceNum * sizeof(MMergeInstanceDrawCallOutput);
		if (m_outputBuffer.GetSize() < nOutputBufferSize)
		{
			m_outputBuffer.ReallocMemory(nOutputBufferSize);
			m_outputBuffer.DestroyBuffer(pRenderSystem->GetDevice());
			m_outputBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nOutputBufferSize);
		}
		if (pStorageParam->pBuffer != &m_outputBuffer)
		{
			pStorageParam->pBuffer = &m_outputBuffer;
			pStorageParam->SetDirty();
		}
	}

	if (std::shared_ptr<MShaderConstantParam>&& pConstantParam = params->FindConstantParam("ubo"))
	{
		MVariantStruct& sut = pConstantParam->var.GetValue<MVariantStruct>();
		{
			MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
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

std::vector<std::shared_ptr<MShaderPropertyBlock>> MShadowMapShaderPropertyBlock::GetPropertyBlock() const
{
	return {
		m_pShaderPropertyBlock,
		m_pMeshInstancePropertyBlock,
	};
}