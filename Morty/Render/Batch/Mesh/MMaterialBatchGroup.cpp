#include "MMaterialBatchGroup.h"
#include "Batch/Texture/ITextureBatcher.h"
#include "Batch/Texture/MTexture2DArrayBatcher.h"
#include "Component/MRenderMeshComponent.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MMaterialTemplate.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Abstract/MIDevice.h"
#include "System/MRenderSystem.h"


using namespace morty;

void MMaterialBatchGroup::Initialize(MIDevice* device, const std::shared_ptr<MShaderParameterSet>& parameterSet, const MShaderPropertyBlock* propertyBlock)
{
    m_device       = device;
    m_parameterSet = parameterSet;

    m_propertyStorage = parameterSet->FindStorageParam(propertyBlock->GetInstancingName(MInstanceDataType::Property));
    m_textureStorage  = parameterSet->FindStorageParam(propertyBlock->GetInstancingName(MInstanceDataType::TextureIndex));
    if (m_propertyStorage) { m_propertyStructSize = m_propertyStorage->var.GetSize(); }
    if (m_textureStorage) { m_textureStructSize = m_textureStorage->var.GetSize(); }

    m_propertyBuffer = MBuffer::CreateStorageBuffer("material batch property buffer");
    m_textureBuffer  = MBuffer::CreateStorageBuffer("material batch texture index buffer");

    for (const auto& texParam: parameterSet->GetTextureParams())
    {
        if (texParam->eType == METextureType::ETexture2DArray)
        {
            auto& batcherData = m_textureBatcherData[texParam->strName] = TextureBatcherData();
            batcherData.batcher                                         = new MTexture2DArrayBatcher();
            batcherData.indexName                                       = MStringId(texParam->strName.ToString() + "Index");
            if (m_textureStorage)
            {
                auto structData = m_textureStorage->var.GetValue<MVariantStruct>();
                auto indexVar   = structData.FindVariant(batcherData.indexName);
                if (indexVar.IsValid())
                {
                    batcherData.indexOffset = indexVar.GetOffset();
                    batcherData.valid       = true;
                }
            }
        }
    }
}

void MMaterialBatchGroup::Release()
{
    for (auto& [name, batcherData]: m_textureBatcherData)
    {
        batcherData.batcher->Release(m_device);
        delete batcherData.batcher;
        batcherData.batcher = nullptr;
    }

    m_textureData.clear();
    m_propertyBuffer.DestroyBuffer(m_device);
    m_textureBuffer.DestroyBuffer(m_device);
}

MMaterialInstanceKey MMaterialBatchGroup::AddInstance(MMeshInstanceKey proxyId, MRenderMeshComponent* component, MMaterial* material)
{
    if (!m_propertyStorage) { return MGlobal::M_INVALID_INDEX; }
    if (!m_textureStorage) { return MGlobal::M_INVALID_INDEX; }

    auto id                  = m_idPool.AllocateID();
    m_instanceTable[proxyId] = id;

    if (m_propertyData.size() < (id + 1) * m_propertyStructSize) { m_propertyData.resize((id + 1) * m_propertyStructSize); }
    if (m_textureData.size() < (id + 1) * m_textureStructSize) { m_textureData.resize((id + 1) * m_textureStructSize); }

    auto instancingData = component->GetInstancingData();
    memcpy(&m_propertyData[id * m_propertyStructSize], instancingData.GetData(), instancingData.GetSize());

    for (const auto& [name, batcherData]: m_textureBatcherData)
    {
        auto texture = material->GetTexture(name);
        if (!texture) continue;

        if (!batcherData.batcher->IsInitialized())
        {
            MTextureBatcherConfig config;
            config.format          = texture->GetFormat();
            config.targetSize      = texture->GetSize2D();
            config.generateMipmaps = texture->GetMipmapLevel() > 1;

            batcherData.batcher->Initialize(m_device, config);
        }
        auto index = batcherData.batcher->RegisterTexture(material->GetTexture(name));
        memcpy(&m_textureData[id * m_textureStructSize + batcherData.indexOffset], &index, sizeof(int));
    }


    m_needSync = true;

    return id;
}

void MMaterialBatchGroup::RemoveInstance(MMeshInstanceKey proxyId)
{
    if (!m_propertyStorage || !m_textureStorage) { return; }

    m_instanceTable.erase(proxyId);
    m_idPool.FreeID(proxyId);
}

MMaterialInstanceKey MMaterialBatchGroup::GetInstanceKey(MMeshInstanceKey proxyId) const
{
    auto it = m_instanceTable.find(proxyId);
    if (it != m_instanceTable.end()) { return it->second; }

    return MGlobal::M_INVALID_INDEX;
}

void MMaterialBatchGroup::RenderThreadUpdate(MIDevice* device)
{
    if (!m_needSync) return;

    m_propertyBuffer.ApplyData(device, m_propertyData);
    m_textureBuffer.ApplyData(device, m_textureData);

    m_propertyStorage->SetBuffer(&m_propertyBuffer);
    m_textureStorage->SetBuffer(&m_textureBuffer);

    for (const auto& [name, batcherData]: m_textureBatcherData) { batcherData.batcher->RenderThreadUpdate(device); }


    m_needSync = false;
}
