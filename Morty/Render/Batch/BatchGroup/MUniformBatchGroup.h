#pragma once

#include "Utility/MGlobal.h"
#include "MInstanceBatchGroup.h"
#include "MRenderInstanceCache.h"
#include "Object/MObject.h"
#include "Shader/MShaderProgram.h"
#include "Variant/MVariant.h"

namespace morty
{

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
class MORTY_API MUniformBatchGroup : public MInstanceBatchGroup
{
public:
    struct MTransformVariant {
        MVariant worldMatrix;
        MVariant normalMatrix;
        MVariant instanceIndex;
    };

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
    std::shared_ptr<MShaderConstantParam>                            m_transformParam      = nullptr;
    MRenderInstanceCache<MMeshInstanceKey, MMeshInstanceRenderProxy> m_instanceCache;

    std::vector<MTransformVariant>                                   m_transformArray;
    size_t                                                           m_currentInstanceNum = 0;
    size_t                                                           m_maxInstanceNum     = 0;
};

}// namespace morty