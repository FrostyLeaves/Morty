#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "MInstanceBatchGroup.h"
#include "MRenderInstanceCache.h"
#include "Object/MObject.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"
#include "Variant/MVariant.h"

namespace morty
{

class MShaderProgram;
struct MShaderConstantParam;

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MRenderMeshComponent;
class MORTY_API MNoneBatchGroup : public MInstanceBatchGroup
{
public:
    void Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram) override;

    void Release(MEngine* pEngine) override;

    bool CanAddMeshInstance() const override;

    void AddMeshInstance(const MMeshInstanceRenderProxy& proxy) override;

    void RemoveMeshInstance(MMeshInstanceKey key) override;

    void UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy) override;

    std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const override { return m_shaderPropertyBlock; }

    MMeshInstanceRenderProxy*             FindMeshInstance(MMeshInstanceKey key) override
    {
        MORTY_UNUSED(key);
        return &m_instance;
    }

    void InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func) override;

private:
    MEngine*                              m_engine              = nullptr;
    std::shared_ptr<MShaderProgram>       m_shaderProgram       = nullptr;
    std::shared_ptr<MShaderPropertyBlock> m_shaderPropertyBlock = nullptr;
    std::shared_ptr<MShaderConstantParam> m_transformParam      = nullptr;

    MMeshInstanceRenderProxy              m_instance;
    bool                                  m_instanceValid = false;
    MVariant                              m_worldMatrix;
    MVariant                              m_normalMatrix;
    MVariant                              m_instanceIdx;
};

}// namespace morty