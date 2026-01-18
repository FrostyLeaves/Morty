/**
 * @File         MMaterialBatchGroup
 *
 * @Created      2025-01-28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MBuffer.h"
#include "Math/Matrix.h"
#include "Utility/MIDPool.h"

#include <memory>

namespace morty
{

class MMaterialTemplate;
class MShaderParameterSet;
class MRenderMeshComponent;
class ITextureBatcher;
class MIDevice;
class MMaterial;
class MShaderPropertyBlock;
class MShaderStorageParam;
struct MTextureBatcherConfig;

// GPU upload data, corresponds to MeshInstanceData on shader side
struct MORTY_API MMeshInstanceRenderProxy {
    Matrix4          worldTransform     = Matrix4::IdentityMatrix;// float4x4 matWorld
    int32_t          meshResourceId     = MGlobal::M_INVALID_INT; // int meshResourceId
    int32_t          batchGroupId       = MGlobal::M_INVALID_INT; // int materialInstanceId
    int32_t          materialInstanceId = MGlobal::M_INVALID_INT;
    MMeshInstanceKey proxyId            = MGlobal::M_INVALID_INDEX;// int proxyId (reserved)
    int32_t          visible            = false;                   // int visible (reserved)
};

// Batch group for mesh instances with the same material template
class MORTY_API MMaterialBatchGroup
{
public:
         MMaterialBatchGroup() = default;
    ~    MMaterialBatchGroup() = default;

    void Initialize(
            MIDevice*                                   device,
            const std::shared_ptr<MShaderParameterSet>& parameterSet,
            const MShaderPropertyBlock*                 propertyBlock
    );

    void                 Release();

    MMaterialInstanceKey AddInstance(MMeshInstanceKey proxyId, MRenderMeshComponent* component, MMaterial* material);
    void                 RemoveInstance(MMeshInstanceKey proxyId);
    [[nodiscard]] MMaterialInstanceKey GetInstanceKey(MMeshInstanceKey proxyId) const;
    void                               SetBatchId(size_t batchId) { m_batchId = batchId; }
    [[nodiscard]] size_t               GetBatchId() const { return m_batchId; }
    [[nodiscard]] size_t               GetInstanceCount() const { return m_instanceTable.size(); }

    void SetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& temp) { m_materialTemplate = temp; }

    [[nodiscard]] std::shared_ptr<MMaterialTemplate>          GetMaterialTemplate() const { return m_materialTemplate; }

    [[nodiscard]] bool                                        IsEmpty() const { return m_instanceTable.size() == 0; }

    [[nodiscard]] const std::shared_ptr<MShaderParameterSet>& GetParameterSet() const { return m_parameterSet; }

    void                                                      RenderThreadUpdate(MIDevice* device);

private:
    size_t                                                     m_batchId            = 0;
    std::shared_ptr<MMaterialTemplate>                         m_materialTemplate   = nullptr;
    MShaderStorageParam*                                       m_propertyStorage    = nullptr;
    size_t                                                     m_propertyStructSize = 0;
    MShaderStorageParam*                                       m_textureStorage     = nullptr;
    size_t                                                     m_textureStructSize  = 0;
    std::unordered_map<MMeshInstanceKey, MMaterialInstanceKey> m_instanceTable;

    MReusableIDPool<MMaterialInstanceKey>                      m_idPool;

    std::shared_ptr<MShaderParameterSet>                       m_parameterSet;
    std::vector<MByte>                                         m_propertyData;
    std::vector<MByte>                                         m_textureData;

    MBuffer                                                    m_propertyBuffer;
    MBuffer                                                    m_textureBuffer;

    bool                                                       m_needSync = false;
    MIDevice*                                                  m_device   = nullptr;

    struct TextureBatcherData {
        ITextureBatcher* batcher = nullptr;
        MStringId        indexName;
        size_t           indexOffset = 0;
        bool             valid       = false;
    };
    std::unordered_map<MStringId, TextureBatcherData> m_textureBatcherData;
};

}// namespace morty
