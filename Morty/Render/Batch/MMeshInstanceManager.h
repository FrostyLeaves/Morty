#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "MMaterialBatchGroup.h"
#include "Material/MMaterial.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"

namespace morty
{

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MTaskNode;
class MMaterial;
class MComponent;
struct MShaderConstantParam;

class MRenderMeshComponent;
class MORTY_API MMeshInstanceManager : public IManager
{
public:
    MORTY_INTERFACE(MMeshInstanceManager)

public:
    void                     Initialize() override;

    void                     Release() override;

    void                     RenderUpdate(MTaskNode* pNode);

    [[nodiscard]] MTaskNode* GetUpdateTask() const { return m_updateTask; }

public:
    void                                            OnMaterialChanged(MComponent* pComponent);

    void                                            OnMeshChanged(MComponent* pComponent);

    void                                            OnSceneComponentChanged(MComponent* pComponent);

    void                                            OnRenderMeshChanged(MComponent* pComponent);

    void                                            RemoveComponent(MRenderMeshComponent* pComponent);

    [[nodiscard]] std::vector<MMaterialBatchGroup*> FindGroupFromMaterialType(MEMaterialType eType) const;

    [[nodiscard]] std::vector<MMaterialBatchGroup*> GetAllMaterialGroup() const;


protected:
    [[nodiscard]] static bool IsRenderableMeshMaterial(MEMaterialType eType);

    void                      AddComponentToGroup(MRenderMeshComponent* pComponent);

    void                      RemoveComponentFromGroup(MRenderMeshComponent* pComponent);

    void                      UpdateMeshInstance(MRenderMeshComponent* pComponent, MMeshInstanceRenderProxy proxy);

    void                      Clean();

private:
    struct MaterialGroup {
        MMaterialBatchGroup                                  materialGroup;
        std::map<MMeshInstanceKey, MMeshInstanceRenderProxy> tWaitUpdateComponent;
        std::set<MMeshInstanceKey>                           tWaitRemoveComponent;
    };

    std::map<MRenderMeshComponent*, MaterialGroup*>              m_componentTable;
    std::map<std::shared_ptr<MMaterialTemplate>, MaterialGroup*> m_renderableMaterialGroup;

    MTaskNode*                                                   m_updateTask = nullptr;
};

}// namespace morty