#include "MNoneBatchGroup.h"

#include "Utility/MGlobal.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Mesh/MVertex.h"
#include "Scene/MEntity.h"
#include "Shader/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

using namespace morty;

void MNoneBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram)
{
    m_engine        = pEngine;
    m_shaderProgram = pShaderProgram;
    MORTY_ASSERT(m_shaderProgram);

    if (m_shaderPropertyBlock)
    {
        const MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
        m_shaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
        m_shaderPropertyBlock = nullptr;
        m_transformParam      = nullptr;
    }

    m_shaderPropertyBlock = MMaterialTemplate::CreateMeshPropertyBlock(pShaderProgram);
    if (m_shaderPropertyBlock)
    {
        m_transformParam    = m_shaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_MESH_MATRIX);
        MVariantStruct& srt = m_transformParam->var.GetValue<MVariantStruct>();
        m_worldMatrix       = srt.FindVariant(MShaderPropertyName::MESH_WORLD_MATRIX);
        m_normalMatrix      = srt.FindVariant(MShaderPropertyName::MESH_NORMAL_MATRIX);
        m_instanceIdx       = srt.FindVariant(MShaderPropertyName::MESH_INSTANCE_INDEX);
    }

    MORTY_ASSERT(m_transformParam);
}

void MNoneBatchGroup::Release(MEngine* pEngine)
{
    const MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
    m_shaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
    m_shaderPropertyBlock = nullptr;
    m_transformParam      = nullptr;
    m_shaderProgram       = nullptr;
}

bool MNoneBatchGroup::CanAddMeshInstance() const { return !m_instanceValid; }

void MNoneBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
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

    if (m_instanceValid)
    {
        MORTY_ASSERT(!m_instanceValid);
        return;
    }

    UpdateMeshInstance(proxy);

    m_instanceValid = true;
}

void MNoneBatchGroup::RemoveMeshInstance(MMeshInstanceKey key)
{
    MORTY_UNUSED(key);
    m_instanceValid = false;
}

void MNoneBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    if (m_worldMatrix.IsValid()) { m_worldMatrix.SetValue(proxy.worldTransform); }

    if (m_normalMatrix.IsValid())
    {
        //Transposed and Inverse.
        Matrix3 matNormal(proxy.worldTransform, 3, 3);

        m_normalMatrix.SetValue(matNormal);
    }
    if (m_instanceIdx.IsValid()) { m_instanceIdx.SetValue(Vector4(proxy.nProxyId, proxy.nSkeletonId, 0, 0)); }

    m_transformParam->SetDirty();
    m_instance = proxy;
}

void MNoneBatchGroup::InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func)
{
    func(m_instance, 0);
}
