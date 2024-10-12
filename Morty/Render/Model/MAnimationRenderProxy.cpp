#include "MAnimationRenderProxy.h"

#include "Engine/MEngine.h"
#include "MSkeleton.h"
#include "MSkeletonInstance.h"
#include "Material/MMaterial.h"
#include "Shader/MShaderParam.h"
#include "Shader/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

using namespace morty;

void MAnimationRenderGroup::AddSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy)
{
    if (m_poseRenderInstance.HasItem(nProxyId))
    {
        MORTY_ASSERT(!m_poseRenderInstance.HasItem(nProxyId));
        return;
    }

    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_poseRenderInstance.AddItem(nProxyId, CreatePoseRenderInstance());
    const size_t nTransformBufferSize = poseProxy.vBoneMatrix.size() * sizeof(Matrix4);
    if (nTransformBufferSize == 0) { return; }

    MemoryInfo memory;
    if (!m_bonesStorageMemoryPool.AllowMemory(nTransformBufferSize, memory))
    {
        const size_t nCurrentMemorySize = m_bonesStorageMemoryPool.GetMaxMemorySize();
        const size_t nAllowMemorySize =
                (std::max)(size_t(nCurrentMemorySize * 1.5f), nCurrentMemorySize + nTransformBufferSize);

        if (m_bonesStorageBuffer.GetSize() < nAllowMemorySize)
        {
            m_bonesStorageBuffer.ResizeMemory(pRenderSystem->GetDevice(), nAllowMemorySize);
        }
        m_bonesStorageMemoryPool.ResizeMemory(nAllowMemorySize);

        MORTY_ASSERT(m_bonesStorageMemoryPool.AllowMemory(nTransformBufferSize, memory));
    }

    const size_t nMatrixOffset      = memory.begin / sizeof(Matrix4);
    const size_t nOffsetBufferBegin = (nProxyId) * sizeof(int32_t);
    const size_t nOffsetBufferSize  = (nProxyId + 1) * sizeof(int32_t);
    if (m_bonesOffsetBuffer.GetSize() < nOffsetBufferSize)
    {
        m_bonesOffsetBuffer.ResizeMemory(pRenderSystem->GetDevice(), nOffsetBufferSize);
    }

    m_bonesOffsetBuffer.UploadBuffer(
            pRenderSystem->GetDevice(),
            nOffsetBufferBegin,
            reinterpret_cast<const MByte*>(&nMatrixOffset),
            sizeof(int32_t)
    );

    if (auto pInstance = m_poseRenderInstance.FindItem(nProxyId))
    {
        pInstance->nMatrixOffset   = memory.begin / sizeof(Matrix4);
        pInstance->bonesMemoryInfo = memory;
    }

    UpdateSkeletonRenderInstance(nProxyId, poseProxy);
}

void MAnimationRenderGroup::RemoveSkeletonRenderInstance(MSkeletonInstanceKey nProxyId)
{
    if (!m_poseRenderInstance.HasItem(nProxyId))
    {
        MORTY_ASSERT(m_poseRenderInstance.HasItem(nProxyId));
        return;
    }

    m_poseRenderInstance.RemoveItem(nProxyId);
}

void MAnimationRenderGroup::UpdateSkeletonRenderInstance(
        MSkeletonInstanceKey    nProxyId,
        const MPoseRenderProxy& poseProxy
)
{
    if (poseProxy.vBoneMatrix.size() == 0) { return; }

    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    auto                 pPoseInstance = m_poseRenderInstance.FindItem(nProxyId);

    if (pPoseInstance->bonesMemoryInfo.size != poseProxy.vBoneMatrix.size() * sizeof(Matrix4))
    {
        MORTY_ASSERT(pPoseInstance->bonesMemoryInfo.size == poseProxy.vBoneMatrix.size() * sizeof(Matrix4));
        return;
    }

    const MemoryInfo& bonesMemoryInfo = pPoseInstance->bonesMemoryInfo;
    m_bonesStorageBuffer.UploadBuffer(
            pRenderSystem->GetDevice(),
            bonesMemoryInfo.begin,
            reinterpret_cast<const MByte*>(poseProxy.vBoneMatrix.data()),
            bonesMemoryInfo.size
    );
}

void MAnimationRenderGroup::UpdateOrCreateMeshInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy)
{
    if (!m_poseRenderInstance.HasItem(nProxyId)) { AddSkeletonRenderInstance(nProxyId, poseProxy); }
    else { UpdateSkeletonRenderInstance(nProxyId, poseProxy); }
}

MAnimationBufferData MAnimationRenderGroup::GetAnimationBuffer() const
{
    return {&m_bonesStorageBuffer.buffer, &m_bonesOffsetBuffer.buffer};
}

void MAnimationRenderGroup::Initialize(MEngine* pEngine)
{
    m_engine = pEngine;

    m_bonesStorageBuffer.buffer.m_memoryType = MBuffer::MMemoryType::EHostVisible;
    m_bonesStorageBuffer.buffer.m_usageType  = MBuffer::MUsageType::EStorage;

    m_bonesOffsetBuffer.buffer.m_memoryType = MBuffer::MMemoryType::EHostVisible;
    m_bonesOffsetBuffer.buffer.m_usageType  = MBuffer::MUsageType::EStorage;

#if MORTY_DEBUG
    m_bonesStorageBuffer.buffer.m_strDebugName = "Storage Batch Skeletal Animation Buffer";
    m_bonesOffsetBuffer.buffer.m_strDebugName  = "Storage Batch Skeletal Offset Buffer";
#endif

    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_bonesStorageBuffer.ResizeMemory(pRenderSystem->GetDevice(), sizeof(Matrix4));
    m_bonesStorageMemoryPool.ResizeMemory(sizeof(Matrix4));
    m_bonesOffsetBuffer.ResizeMemory(pRenderSystem->GetDevice(), sizeof(int32_t));

    auto                       pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::shared_ptr<MResource> pMeshVSResource = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
    std::shared_ptr<MResource> pMeshPSResource = pResourceSystem->LoadResource("Shader/Deferred/deferred_gbuffer.mps");

    auto pShaderProgram = MShaderProgram::MakeShared(GetEngine(), MShaderProgram::EUsage::EGraphics);
    pShaderProgram->GetShaderMacro().AddUnionMacro(
            MRenderGlobal::SHADER_SKELETON_ENABLE,
            MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG
    );
    pShaderProgram->LoadShader(pMeshVSResource);
    pShaderProgram->LoadShader(pMeshPSResource);

    m_shaderPropertyBlock = MMaterialTemplate::CreateMeshPropertyBlock(pShaderProgram);
    if (m_shaderPropertyBlock)
    {
        if (auto pBoneProperty = m_shaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_BONES_MATRIX))
        {
            pBoneProperty->pBuffer = &m_bonesStorageBuffer.buffer;
        }

        if (auto pBoneOffset = m_shaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_BONES_OFFSET))
        {
            pBoneOffset->pBuffer = &m_bonesOffsetBuffer.buffer;
        }
    }
}

void MAnimationRenderGroup::Release(MEngine* pEngine)
{
    m_poseRenderInstance               = {};
    const MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
    m_bonesStorageBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
    m_bonesOffsetBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
}

MPoseRenderProxy MAnimationRenderGroup::CreatePoseProxy(MSkeletonInstance* pSkeletonInstance)
{
    MPoseRenderProxy     resultPose;

    const MSkeletonPose& pose = pSkeletonInstance->GetCurrentPose();
    size_t               size = pose.vBoneMatrix.size();
    if (size > MRenderGlobal::BONES_MAX_NUMBER) { size = MRenderGlobal::BONES_MAX_NUMBER; }
    resultPose.vBoneMatrix.resize(size);

    memcpy(resultPose.vBoneMatrix.data(), pose.vBoneMatrix.data(), size * sizeof(Matrix4));
    return resultPose;
}

MPoseRenderInstance MAnimationRenderGroup::CreatePoseRenderInstance() { return MPoseRenderInstance(); }
