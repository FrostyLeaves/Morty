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

struct MORTY_API MMeshInstanceRenderProxy {
    bool                 visible        = false;
    MMeshInstanceKey     proxyId        = MGlobal::M_INVALID_INDEX;
    MMaterialInstanceKey materialId     = MGlobal::M_INVALID_INDEX;
    Matrix4              worldTransform = Matrix4::IdentityMatrix;
};

struct MORTY_API MeshInstanceRenderData {
    Matrix4  worldTransform = Matrix4::IdentityMatrix;
    Matrix3  normalMatrix   = Matrix3::IdentityMatrix;
    uint32_t clusterIdx     = 0;
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

    void SetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& temp) { m_materialTemplate = temp; }

    [[nodiscard]] std::shared_ptr<MMaterialTemplate>          GetMaterialTemplate() const { return m_materialTemplate; }

    [[nodiscard]] bool                                        IsEmpty() const { return m_materialTable.size() == 0; }

    [[nodiscard]] const std::shared_ptr<MShaderParameterSet>& GetParameterSet() const { return m_parameterSet; }

private:
    std::shared_ptr<MMaterialTemplate>                         m_materialTemplate = nullptr;
    std::unordered_map<MMeshInstanceKey, MMaterialInstanceKey> m_materialTable;

    MReusableIDPool<MMaterialInstanceKey>                      m_idPool;

    std::shared_ptr<MShaderParameterSet>                       m_parameterSet;
    size_t                                                     m_instanceDataSize;

    MBuffer                                                    m_materialData;
};

}// namespace morty
