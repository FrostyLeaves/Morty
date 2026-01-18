#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Container/MItemPool.h"
#include "MMaterialBatchGroup.h"
#include "Material/MMaterial.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"

#include <mutex>
#include <unordered_map>
#include <vector>


namespace morty
{

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MTaskNode;
class MMaterial;
class MMaterialTemplate;
class MComponent;
struct MShaderConstantParam;
class MRenderMeshComponent;

class MORTY_API MMeshInstanceManager : public IManager
{
public:
    MORTY_INTERFACE(MMeshInstanceManager)

public:
    void                                     Initialize() override;

    void                                     Release() override;

    [[nodiscard]] std::set<const MType*>     RegisterComponentType() const override;

    void                                     UnregisterComponent(MComponent* component) override;

    void                                     SceneTick(MScene* scene, const float& delta) override;

    void                                     RenderUpdate(MTaskNode* node);

    [[nodiscard]] MTaskNode*                 GetUpdateTask() const { return m_updateTask; }

    void                                     OnMaterialChanged(MComponent* component);

    void                                     OnMeshChanged(MComponent* component);

    void                                     OnSceneComponentChanged(MComponent* component);

    void                                     OnRenderMeshChanged(MComponent* component);

    void                                     OnRemoveComponent(MRenderMeshComponent* component);

    const std::vector<MMaterialBatchGroup*>& GetBatchGroups() const { return m_renderData.batchGroups; }

    const MBuffer*                           GetInstanceBuffer() const { return &m_renderData.instanceBuffer; }

protected:
    void                     AddComponentToGroup(MRenderMeshComponent* component);
    void                     RemoveComponentFromGroup(MRenderMeshComponent* component);
    void                     UpdateMeshInstance(MRenderMeshComponent* component, MMeshInstanceRenderProxy proxy);
    void                     Clean();

    MMeshInstanceRenderProxy CreateProxyFromComponent(MRenderMeshComponent* component);
    void                     UpdateStorageBufferIfNeeded();

private:
    // Main thread data - for receiving notifications and updates
    struct MainThreadData {
        // Mapping from component ID to material template for fast group lookup
        std::unordered_map<MMeshInstanceKey, std::shared_ptr<MMaterialTemplate>> componentToMaterialTemplate;
        // Cache of batch groups created on main thread
        std::unordered_map<std::shared_ptr<MMaterialTemplate>, std::unique_ptr<MMaterialBatchGroup>>
                                        materialTemplateToBatchGroup;

        MItemPool<MMaterialBatchGroup*> batchGroups;
    };

    // Render thread data - for rendering
    struct RenderThreadData {
        std::vector<MMaterialBatchGroup*>     batchGroups;
        std::vector<MMeshInstanceRenderProxy> renderProxies;
        MBuffer                               instanceBuffer;
    };

    // Update queue, written by main thread and read by render thread
    struct UpdateCommand {
        enum class Type
        {
            AddGroup,
            RemoveGroup,
            AddInstance,
            RemoveInstance,
            UpdateInstance
        };

        Type                     type;
        size_t                   batchId;
        MMeshInstanceKey         proxyId;
        MMeshInstanceRenderProxy proxy;
        MMaterialBatchGroup*     batchGroup = nullptr;// Pre-created batch group from main thread
    };

    MTaskNode*                 m_updateTask = nullptr;

    MainThreadData             m_mainThreadData;
    RenderThreadData           m_renderData;

    std::mutex                 m_updateMutex;
    std::vector<UpdateCommand> m_updateQueue;
    std::vector<UpdateCommand> m_pendingCommands;
};

}// namespace morty