#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
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
    void                     Initialize() override;

    void                     Release() override;

    std::set<const MType*>   RegisterComponentType() const override;

    void                     UnregisterComponent(MComponent* component) override;

    void                     RenderUpdate(MTaskNode* pNode);

    [[nodiscard]] MTaskNode* GetUpdateTask() const { return m_updateTask; }

public:
    void OnMaterialChanged(MComponent* component);

    void OnMeshChanged(MComponent* component);

    void OnSceneComponentChanged(MComponent* component);

    void OnRenderMeshChanged(MComponent* component);

    void RemoveComponent(MRenderMeshComponent* component);

    [[nodiscard]] const std::unordered_map<std::shared_ptr<MMaterialTemplate>, std::shared_ptr<MMaterialBatchGroup>>&
    GetBatchGroups() const
    {
        return m_renderData.batchGroups;
    }

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
    };

    // Render thread data - for rendering
    struct RenderThreadData {
        // Batch groups organized by material template
        std::unordered_map<std::shared_ptr<MMaterialTemplate>, std::shared_ptr<MMaterialBatchGroup>> batchGroups;

        // Fast lookup from ProxyId to BatchGroup
        std::unordered_map<MMeshInstanceKey, std::shared_ptr<MMaterialTemplate>> proxyToMaterialTemplate;

        std::vector<MMeshInstanceRenderProxy>                                    renderProxies;

        MBuffer                                                                  instanceBuffer;
    };

    // Update queue, written by main thread and read by render thread
    struct UpdateCommand {
        enum class Type
        {
            Add,
            Remove,
            Update
        };

        Type                               type;
        MMeshInstanceKey                   proxyId;
        MMeshInstanceRenderProxy           proxy;
        std::shared_ptr<MMaterialTemplate> materialTemplate;
    };

    MTaskNode*                 m_updateTask = nullptr;

    MainThreadData             m_mainThreadData;
    RenderThreadData           m_renderData;

    std::mutex                 m_updateMutex;
    std::vector<UpdateCommand> m_updateQueue;
};

}// namespace morty