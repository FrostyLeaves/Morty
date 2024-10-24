#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Material/MMaterial.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"

#include "Basic/MStorageVariant.h"
#include "Batch/BatchGroup/MRenderInstanceCache.h"
#include "Batch/MMaterialBatchGroup.h"
#include "Component/MRenderMeshComponent.h"
#include "Shadow/MShadowMapUtil.h"

namespace morty
{

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MTaskNode;
class MComponent;
class MComputeDispatcher;
class MShaderPropertyBlock;
class MRenderMeshComponent;
struct MShaderConstantParam;


class MORTY_API MShadowMeshManager : public IManager
{
public:
    MORTY_CLASS(MShadowMeshManager)

    struct MShadowMeshInstance {
        MIMesh*       pMesh = nullptr;
        MemoryInfo    transformBuffer;
        MemoryInfo    cullingBuffer;
        MBoundsSphere boundsSphere;
        MBoundsSphere boundsInWorld;
    };

public:
    void                              Initialize() override;

    void                              Release() override;

    std::set<const MType*>            RegisterComponentType() const override;

    void                              UnregisterComponent(MComponent* pComponent) override;

    void                              RenderUpdate(MTaskNode* pNode);

    MTaskNode*                        GetUpdateTask() const { return m_updateTask; }

    void                              OnTransformChanged(MComponent* pComponent);

    void                              OnMeshChanged(MComponent* pComponent);

    void                              OnGenerateShadowChanged(MComponent* pComponent);

    std::vector<MMaterialBatchGroup*> GetAllShadowGroup() const;

protected:
    void AddToUpdateQueue(MRenderMeshComponent* pComponent);

    void AddToDeleteQueue(MRenderMeshComponent* pComponent);

    void InitializeMaterial();

    void ReleaseMaterial();

private:
    struct MaterialGroup {
        MMaterialBatchGroup                                  materialGroup;
        std::map<MMeshInstanceKey, MMeshInstanceRenderProxy> tWaitUpdateComponent;
        std::set<MMeshInstanceKey>                           tWaitRemoveComponent;
    };
    std::map<MEMeshVertexType, MaterialGroup*>      m_batchMaterialGroup;
    std::map<MRenderMeshComponent*, MaterialGroup*> m_componentTable;
    std::vector<MMaterialBatchGroup*>               m_materialGroup;

    MResourceRef                                    m_staticMaterial;
    MResourceRef                                    m_animatedMaterial;

    MTaskNode*                                      m_updateTask = nullptr;
};

}// namespace morty