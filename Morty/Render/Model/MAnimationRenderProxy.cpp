#include "MAnimationRenderProxy.h"

#include "MSkeleton.h"
#include "Engine/MEngine.h"
#include "MSkeletonInstance.h"
#include "Material/MMaterial.h"
#include "Material/MShaderParam.h"
#include "Material/MShaderPropertyBlock.h"
#include "Resource/MMaterialResource.h"
#include "System/MRenderSystem.h"

void MAnimationRenderGroup::AddSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy)
{
	if (m_tPoseRenderInstance.HasItem(nProxyId))
	{
		MORTY_ASSERT(!m_tPoseRenderInstance.HasItem(nProxyId));
		return;
	}

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_tPoseRenderInstance.AddItem(nProxyId, CreatePoseRenderInstance());
	const size_t nTransformBufferSize = poseProxy.vBoneMatrix.size() * sizeof(Matrix4);
	if (nTransformBufferSize == 0)
	{
		return;
	}

	MemoryInfo memory;
	if (!m_bonesStorageMemoryPool.AllowMemory(nTransformBufferSize, memory))
	{
		const size_t nCurrentMemorySize = m_bonesStorageMemoryPool.GetMaxMemorySize();
		const size_t nAllowMemorySize = (std::max)(size_t(nCurrentMemorySize * 1.5f), nCurrentMemorySize + nTransformBufferSize);

		if (m_bonesStorageBuffer.GetSize() < nAllowMemorySize)
		{
			m_bonesStorageBuffer.ResizeMemory(pRenderSystem->GetDevice(), nAllowMemorySize);
		}
		m_bonesStorageMemoryPool.ResizeMemory(nAllowMemorySize);

		MORTY_ASSERT(m_bonesStorageMemoryPool.AllowMemory(nTransformBufferSize, memory));
	}

	const int32_t nMatrixOffset = memory.begin / sizeof(Matrix4);
	const size_t nOffsetBufferBegin = (nProxyId) * sizeof(int32_t);
	const size_t nOffsetBufferSize = (nProxyId + 1) * sizeof(int32_t);
	if (m_bonesOffsetBuffer.GetSize() < nOffsetBufferSize)
	{
		m_bonesOffsetBuffer.ResizeMemory(pRenderSystem->GetDevice(), nOffsetBufferSize);
	}

	m_bonesOffsetBuffer.UploadBuffer(pRenderSystem->GetDevice(), nOffsetBufferBegin, reinterpret_cast<const MByte*>(&nMatrixOffset), sizeof(int32_t));

	if (auto pInstance = m_tPoseRenderInstance.FindItem(nProxyId))
	{
		pInstance->nMatrixOffset = memory.begin / sizeof(Matrix4);
		pInstance->bonesMemoryInfo = memory;
	}

	UpdateSkeletonRenderInstance(nProxyId, poseProxy);
}

void MAnimationRenderGroup::RemoveSkeletonRenderInstance(MSkeletonInstanceKey nProxyId)
{
	if (!m_tPoseRenderInstance.HasItem(nProxyId))
	{
		MORTY_ASSERT(m_tPoseRenderInstance.HasItem(nProxyId));
		return;
	}

	m_tPoseRenderInstance.RemoveItem(nProxyId);
}

void MAnimationRenderGroup::UpdateSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy)
{
	if (poseProxy.vBoneMatrix.size() == 0)
	{
		return;
	}

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	auto pPoseInstance = m_tPoseRenderInstance.FindItem(nProxyId);

	if (pPoseInstance->bonesMemoryInfo.size != poseProxy.vBoneMatrix.size() * sizeof(Matrix4))
	{
		MORTY_ASSERT(pPoseInstance->bonesMemoryInfo.size == poseProxy.vBoneMatrix.size() * sizeof(Matrix4));
		return;
	}

	const MemoryInfo& bonesMemoryInfo = pPoseInstance->bonesMemoryInfo;
	m_bonesStorageBuffer.UploadBuffer(pRenderSystem->GetDevice(), bonesMemoryInfo.begin, reinterpret_cast<const MByte*>(poseProxy.vBoneMatrix.data()), bonesMemoryInfo.size);
}

void MAnimationRenderGroup::UpdateOrCreateMeshInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy)
{
	if (!m_tPoseRenderInstance.HasItem(nProxyId))
	{
		AddSkeletonRenderInstance(nProxyId, poseProxy);
	}
    else
    {
		UpdateSkeletonRenderInstance(nProxyId, poseProxy);
    }
}

void MAnimationRenderGroup::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_bonesStorageBuffer.buffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_bonesStorageBuffer.buffer.m_eUsageType = MBuffer::MUsageType::EStorage;

	m_bonesOffsetBuffer.buffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_bonesOffsetBuffer.buffer.m_eUsageType = MBuffer::MUsageType::EStorage;

#if MORTY_DEBUG
	m_bonesStorageBuffer.buffer.m_strDebugBufferName = "Storage Batch Skeletal Animation Buffer";
	m_bonesOffsetBuffer.buffer.m_strDebugBufferName = "Storage Batch Skeletal Offset Buffer";
#endif

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_bonesStorageBuffer.ResizeMemory(pRenderSystem->GetDevice(), sizeof(Matrix4));
	m_bonesStorageMemoryPool.ResizeMemory(sizeof(Matrix4));
	m_bonesOffsetBuffer.ResizeMemory(pRenderSystem->GetDevice(), sizeof(int32_t));

	auto pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
  
	std::shared_ptr<MResource> pMeshVSResource = pResourceSystem->LoadResource("Shader/Deferred/model_gbuffer.mvs");
	std::shared_ptr<MResource> pMeshPSResource = pResourceSystem->LoadResource("Shader/Deferred/model_gbuffer.mps");
	
	std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->GetShaderMacro().SetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE, "1");
	pMaterial->LoadVertexShader(pMeshVSResource);
	pMaterial->LoadPixelShader(pMeshPSResource);

	if (std::shared_ptr<MShaderPropertyBlock> pTemplatePropertyBlock = pMaterial->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_SKELETON])
	{
		m_pShaderPropertyBlock = pTemplatePropertyBlock->Clone();
		if (auto pBoneProperty = m_pShaderPropertyBlock->FindStorageParam("u_vBonesMatrix"))
		{
			pBoneProperty->pBuffer = &m_bonesStorageBuffer.buffer;
		}

		if (auto pBoneOffset = m_pShaderPropertyBlock->FindStorageParam("u_vBonesOffset"))
		{
			pBoneOffset->pBuffer = &m_bonesOffsetBuffer.buffer;
		}
	}

	pResourceSystem->UnloadResource(pMaterial);
}

void MAnimationRenderGroup::Release(MEngine* pEngine)
{
	m_tPoseRenderInstance = {};
	const MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	m_bonesStorageBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_bonesOffsetBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
}

MPoseRenderProxy MAnimationRenderGroup::CreatePoseProxy(MSkeletonInstance* pSkeletonInstance)
{
	MPoseRenderProxy resultPose;

	const MSkeletonPose& pose = pSkeletonInstance->GetCurrentPose();
	uint32_t size = pose.vBoneMatrix.size();
	if (size > MRenderGlobal::BONES_MAX_NUMBER)
	{
		size = MRenderGlobal::BONES_MAX_NUMBER;
	}
	resultPose.vBoneMatrix.resize(size);

	memcpy(resultPose.vBoneMatrix.data(), pose.vBoneMatrix.data(), size * sizeof(Matrix4));
	return resultPose;
}

MPoseRenderInstance MAnimationRenderGroup::CreatePoseRenderInstance()
{
	return MPoseRenderInstance();
}
