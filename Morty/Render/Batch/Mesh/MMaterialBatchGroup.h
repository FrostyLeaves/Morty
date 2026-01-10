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

namespace morty
{

class MMaterialTemplate;
class MShaderParameterSet;
class MRenderMeshComponent;

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
     MMaterialBatchGroup(const std::shared_ptr<MShaderParameterSet>& parameterSet, const MStringId& name);
    ~MMaterialBatchGroup() = default;

    MMaterialInstanceKey               AddInstance(MMeshInstanceKey proxyId);

    void                               RemoveInstance(MMeshInstanceKey proxyId);

    [[nodiscard]] MMaterialInstanceKey GetInstanceKey(MMeshInstanceKey proxyId) const;

    void                               SetBatchId(size_t batchId) { m_batchId = batchId; }
    [[nodiscard]] size_t               GetBatchId() const { return m_batchId; }
    [[nodiscard]] size_t               GetInstanceCount() const { return m_instanceTable.size(); }

    void SetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& temp) { m_materialTemplate = temp; }

    [[nodiscard]] std::shared_ptr<MMaterialTemplate>          GetMaterialTemplate() const { return m_materialTemplate; }

    [[nodiscard]] bool                                        IsEmpty() const { return m_instanceTable.size() == 0; }

    [[nodiscard]] const std::shared_ptr<MShaderParameterSet>& GetParameterSet() const { return m_parameterSet; }

private:
    size_t                                                     m_batchId          = 0;
    std::shared_ptr<MMaterialTemplate>                         m_materialTemplate = nullptr;
    std::unordered_map<MMeshInstanceKey, MMaterialInstanceKey> m_instanceTable;

    MReusableIDPool<MMaterialInstanceKey>                      m_idPool;

    std::shared_ptr<MShaderParameterSet>                       m_parameterSet;
    size_t                                                     m_instanceDataSize;

    MBuffer                                                    m_materialData;
};

}// namespace morty
