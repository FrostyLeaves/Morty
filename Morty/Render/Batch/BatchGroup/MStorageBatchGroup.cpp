#include "MStorageBatchGroup.h"

#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Scene/MEntity.h"
#include "Shader/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Mesh/MVertex.h"

using namespace morty;

constexpr size_t TransformStructSize = sizeof(MMeshInstanceTransform);

void             MStorageBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram)
{
    m_engine        = pEngine;
    m_shaderProgram = pShaderProgram;
    if (m_shaderPropertyBlock)
    {
        const MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
        m_shaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
        m_shaderPropertyBlock = nullptr;
        m_transformParam      = nullptr;
    }

    MORTY_ASSERT(pShaderProgram);

    m_shaderPropertyBlock = MMaterialTemplate::CreateMeshPropertyBlock(pShaderProgram);
    MORTY_ASSERT(m_shaderPropertyBlock);

    m_transformParam = m_shaderPropertyBlock->FindStorageParam(MShaderPropertyName::CBUFFER_MESH_MATRIX);
    MORTY_ASSERT(m_transformParam);

    m_transformBuffer.buffer.m_memoryType = MBuffer::MMemoryType::EHostVisible;
    m_transformBuffer.buffer.m_usageType  = MBuffer::MUsageType::EStorage;
#if MORTY_DEBUG
    m_transformBuffer.buffer.m_strDebugName = "Storage Batch Instance Transform Buffer";
#endif

    m_transformParam->pBuffer = &m_transformBuffer.buffer;
}

void MStorageBatchGroup::Release(MEngine* pEngine)
{
    const MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
    m_shaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
    m_shaderPropertyBlock = nullptr;
    m_transformParam      = nullptr;
    m_shaderProgram       = nullptr;
    m_instanceCache       = {};


    m_transformBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
}

bool MStorageBatchGroup::CanAddMeshInstance() const { return true; }

void MStorageBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    auto key = proxy.nProxyId;
    if (key == MGlobal::M_INVALID_UINDEX)
    {
        MORTY_ASSERT(key != MGlobal::M_INVALID_UINDEX);
        return;
    }

    if (!m_transformParam)
    {
        MORTY_ASSERT(m_transformParam);
        return;
    }

    if (m_instanceCache.HasItem(key))
    {
        MORTY_ASSERT(false);
        return;
    }

    const MRenderSystem* pRenderSystem        = m_engine->FindSystem<MRenderSystem>();
    const size_t         nCurrentIdx          = m_instanceCache.AddItem(key, {});
    const size_t         nItemNum             = m_instanceCache.GetItems().size();
    const size_t         nTransformBufferSize = nItemNum * TransformStructSize;
    if (m_transformBuffer.GetSize() < nTransformBufferSize)
    {
        m_transformBuffer.ResizeMemory(pRenderSystem->GetDevice(), nTransformBufferSize);
    }

    if (m_transformArray.size() < nItemNum) { m_transformArray.resize(nItemNum); }

    m_transformArray[nCurrentIdx].begin = nCurrentIdx * TransformStructSize;
    m_transformArray[nCurrentIdx].size  = TransformStructSize;

    UpdateMeshInstance(proxy);
}

void MStorageBatchGroup::RemoveMeshInstance(MMeshInstanceKey key)
{
    const auto pInstance = m_instanceCache.FindItem(key);
    if (nullptr == pInstance)
    {
        MORTY_ASSERT(false);
        return;
    }

    m_instanceCache.RemoveItem(key);
}

void MStorageBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    auto       key       = proxy.nProxyId;
    const auto pInstance = m_instanceCache.FindItem(key);
    if (nullptr == pInstance) { return; }

    const MRenderSystem*   pRenderSystem   = m_engine->FindSystem<MRenderSystem>();
    const size_t           nCurrentIdx     = m_instanceCache.GetItemIdx(key);
    const auto&            transformMemory = m_transformArray[nCurrentIdx];

    MMeshInstanceTransform data;
    data.transform       = proxy.worldTransform;
    data.normalTransform = Matrix3(data.transform, 3, 3);
    data.instanceIndex.x = proxy.nProxyId;
    data.instanceIndex.y = proxy.nSkeletonId;
    m_transformBuffer.UploadBuffer(
            pRenderSystem->GetDevice(),
            transformMemory.begin,
            reinterpret_cast<const MByte*>(&data),
            transformMemory.size
    );

    *pInstance = proxy;
}

MMeshInstanceRenderProxy* MStorageBatchGroup::FindMeshInstance(MMeshInstanceKey key)
{
    auto result = m_instanceCache.FindItem(key);
    return result;
}

void MStorageBatchGroup::InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func)
{
    const auto& items = m_instanceCache.GetItems();
    for (size_t nIdx = 0; nIdx < items.size(); ++nIdx)
    {
        if (items[nIdx].bValid) { func(items[nIdx].value, nIdx); }
    }
}
