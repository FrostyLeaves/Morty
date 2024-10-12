#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Basic/MStorageVariant.h"
#include "MInstanceBatchGroup.h"
#include "MRenderInstanceCache.h"
#include "Object/MObject.h"
#include "Shader/MShaderParam.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

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
class MShaderProgram;
class MSceneComponent;
class MRenderMeshComponent;
class MORTY_API MStorageBatchGroup : public MInstanceBatchGroup
{
public:
    void Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram) override;

    void Release(MEngine* pEngine) override;

    bool CanAddMeshInstance() const override;

    void AddMeshInstance(const MMeshInstanceRenderProxy& proxy) override;

    void RemoveMeshInstance(MMeshInstanceKey key) override;

    void UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy) override;

    std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const override { return m_shaderPropertyBlock; }

    MMeshInstanceRenderProxy*             FindMeshInstance(MMeshInstanceKey key) override;

    void InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func) override;

private:
    MEngine*                                                         m_engine              = nullptr;
    std::shared_ptr<MShaderProgram>                                  m_shaderProgram       = nullptr;
    std::shared_ptr<MShaderPropertyBlock>                            m_shaderPropertyBlock = nullptr;
    std::shared_ptr<MShaderStorageParam>                             m_transformParam      = nullptr;

    std::vector<MemoryInfo>                                          m_transformArray;
    MRenderInstanceCache<MMeshInstanceKey, MMeshInstanceRenderProxy> m_instanceCache;
    MStorageVariant                                                  m_transformBuffer;
};

}// namespace morty