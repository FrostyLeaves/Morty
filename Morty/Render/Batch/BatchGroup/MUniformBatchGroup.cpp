#include "MUniformBatchGroup.h"

#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Scene/MEntity.h"
#include "Shader/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

#include "Utility/MGlobal.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Mesh/MVertex.h"

using namespace morty;

void morty::MUniformBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram)
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
    if (m_shaderPropertyBlock)
    {
        m_transformParam = m_shaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_MESH_MATRIX);
    }

    MORTY_ASSERT(m_transformParam);

    MStruct&       meshMatrixCbuffer = m_transformParam->var.GetValue<MStruct>();
    MVariantArray& arr = meshMatrixCbuffer.GetVariant<MVariantArray>(MShaderPropertyName::MESH_LOCAL_MATRIX);
    m_maxInstanceNum   = arr.MemberNum();

    m_transformArray.resize(m_maxInstanceNum);
    for (size_t nIdx = 0; nIdx < m_maxInstanceNum; ++nIdx)
    {
        MVariantStruct& srt                  = arr[nIdx].GetValue<MVariantStruct>();
        m_transformArray[nIdx].worldMatrix   = srt.FindVariant(MShaderPropertyName::MESH_WORLD_MATRIX);
        m_transformArray[nIdx].normalMatrix  = srt.FindVariant(MShaderPropertyName::MESH_NORMAL_MATRIX);
        m_transformArray[nIdx].instanceIndex = srt.FindVariant(MShaderPropertyName::MESH_INSTANCE_INDEX);
    }
}

void MUniformBatchGroup::Release(MEngine* pEngine)
{
    const MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
    m_shaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
    m_shaderPropertyBlock = nullptr;
    m_transformParam      = nullptr;
    m_shaderProgram       = nullptr;
    m_currentInstanceNum  = 0;
    m_maxInstanceNum      = 1;

    m_instanceCache = {};
}

bool MUniformBatchGroup::CanAddMeshInstance() const { return m_currentInstanceNum < m_maxInstanceNum; }

void MUniformBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    auto key = proxy.nProxyId;

    if (key == MGlobal::M_INVALID_UINDEX)
    {
        MORTY_ASSERT(key);
        return;
    }

    if (!m_transformParam)
    {
        MORTY_ASSERT(m_transformParam);
        return;
    }

    if (m_currentInstanceNum >= m_maxInstanceNum)
    {
        MORTY_ASSERT(m_currentInstanceNum < m_maxInstanceNum);
        return;
    }

    if (m_instanceCache.HasItem(key))
    {
        MORTY_ASSERT(false);
        return;
    }

    m_instanceCache.AddItem(key, {});
    ++m_currentInstanceNum;


    UpdateMeshInstance(proxy);
}

void MUniformBatchGroup::RemoveMeshInstance(MMeshInstanceKey key)
{
    const auto pInstance = m_instanceCache.FindItem(key);
    if (nullptr == pInstance)
    {
        MORTY_ASSERT(false);
        return;
    }

    m_instanceCache.RemoveItem(key);
    --m_currentInstanceNum;
}

void MUniformBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    auto       key       = proxy.nProxyId;
    const auto pInstance = m_instanceCache.FindItem(key);
    if (nullptr == pInstance) { return; }

    size_t  nIdx = m_instanceCache.GetItemIdx(key);

    //Transposed and Inverse.
    Matrix3 matNormal(proxy.worldTransform, 3, 3);
    m_transformArray[nIdx].normalMatrix.SetValue(matNormal);
    m_transformArray[nIdx].worldMatrix.SetValue(proxy.worldTransform);
    m_transformArray[nIdx].instanceIndex.SetValue(Vector4(proxy.nProxyId, proxy.nSkeletonId, 0, 0));
    m_transformParam->SetDirty();

    *pInstance = proxy;
}

MMeshInstanceRenderProxy* MUniformBatchGroup::FindMeshInstance(MMeshInstanceKey key)
{
    auto result = m_instanceCache.FindItem(key);
    return result;
}

void MUniformBatchGroup::InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func)
{
    const auto& items = m_instanceCache.GetItems();
    for (size_t nIdx = 0; nIdx < items.size(); ++nIdx)
    {
        if (items[nIdx].bValid) { func(items[nIdx].value, nIdx); }
    }
}
