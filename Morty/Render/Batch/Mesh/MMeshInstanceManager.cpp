#include "MMeshInstanceManager.h"

#include "Utility/MGlobal.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MMaterialBatchGroup.h"
#include "MRenderNotify.h"
#include "Material/MMaterialPass.h"
#include "Material/MMaterialTemplate.h"
#include "Mesh/MMeshManager.h"
#include "Module/MCoreNotify.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Shader/MShaderProgram.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"


using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MMeshInstanceManager, IManager)

std::set<const MType*> MMeshInstanceManager::RegisterComponentType() const
{
    return {MRenderMeshComponent::GetClassType()};
}

void MMeshInstanceManager::Initialize()
{
    Super::Initialize();

    if (MNotifyManager* notifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        notifySystem->RegisterNotify(
                MCoreNotify::NOTIFY_VISIBLE_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this)
        );
        notifySystem->RegisterNotify(
                MCoreNotify::NOTIFY_TRANSFORM_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this)
        );
        notifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_ATTACHED_SKELETON_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnRenderMeshChanged, this)
        );
        notifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_MATERIAL_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMaterialChanged, this)
        );
        notifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_MESH_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMeshChanged, this)
        );
    }

    m_updateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>(MRenderGlobal::TASK_RENDER_MESH_MANAGER_UPDATE);
    if (m_updateTask)
    {
        m_updateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::RenderUpdate, this));
        m_updateTask->SetThreadType(METhreadType::ERenderThread);
    }


    m_renderData.instanceBuffer = MBuffer::CreateStorageBuffer("MeshInstanceManager::InstanceBuffer");
}

void MMeshInstanceManager::Release()
{
    if (m_updateTask)
    {
        GetEngine()->GetMainGraph()->DestroyNode(m_updateTask);
        m_updateTask = nullptr;
    }

    if (MNotifyManager* notifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        notifySystem->UnregisterNotify(
                MCoreNotify::NOTIFY_VISIBLE_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this)
        );
        notifySystem->UnregisterNotify(
                MCoreNotify::NOTIFY_TRANSFORM_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this)
        );
        notifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_ATTACHED_SKELETON_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnRenderMeshChanged, this)
        );
        notifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_MATERIAL_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMaterialChanged, this)
        );
        notifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_MESH_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMeshChanged, this)
        );
    }

    Clean();

    auto renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    m_renderData.instanceBuffer.DestroyBuffer(renderSystem->GetDevice());

    Super::Release();
}

void MMeshInstanceManager::UnregisterComponent(MComponent* component)
{
    if (auto* meshComponent = component->template DynamicCast<MRenderMeshComponent>())
    {
        OnRemoveComponent(meshComponent);
    }
}

void MMeshInstanceManager::SceneTick(MScene* scene, const float& delta)
{
    MORTY_UNUSED(scene);
    MORTY_UNUSED(delta);

    {
        std::lock_guard<std::mutex> lock(m_updateMutex);
        if (m_pendingCommands.empty()) { return; }
        m_pendingCommands.swap(m_updateQueue);
    }
}

void MMeshInstanceManager::RenderUpdate(MTaskNode* node)
{
    MORTY_UNUSED(node);


    // Process update queue
    std::vector<UpdateCommand> commands;
    {
        std::lock_guard<std::mutex> lock(m_updateMutex);
        if (m_updateQueue.empty()) { return; }
        commands.swap(m_updateQueue);
    }


    bool storageBufferNeedsUpdate = false;

    // Process all commands on render thread
    for (const auto& cmd: commands)
    {
        switch (cmd.type)
        {
            case UpdateCommand::Type::AddGroup: {
                if (!cmd.batchGroup) { break; }
                storageBufferNeedsUpdate = true;

                m_renderData.batchGroups.push_back(cmd.batchGroup);
                break;
            }
            case UpdateCommand::Type::RemoveGroup: {
                if (!cmd.batchGroup) { break; }
                storageBufferNeedsUpdate = true;

                auto it = std::find(m_renderData.batchGroups.begin(), m_renderData.batchGroups.end(), cmd.batchGroup);
                if (it != m_renderData.batchGroups.end()) { m_renderData.batchGroups.erase(it); }
                break;
            }
            case UpdateCommand::Type::AddInstance: {
                if (!cmd.batchGroup) { break; }
                storageBufferNeedsUpdate = true;

                if (m_renderData.renderProxies.size() <= cmd.proxy.proxyId)
                {
                    m_renderData.renderProxies.resize(cmd.proxy.proxyId + 1);
                }
                m_renderData.renderProxies[cmd.proxy.proxyId] = cmd.proxy;
                break;
            }
            case UpdateCommand::Type::RemoveInstance: {
                storageBufferNeedsUpdate                = true;
                m_renderData.renderProxies[cmd.proxyId] = MMeshInstanceRenderProxy();
                break;
            }
            case UpdateCommand::Type::UpdateInstance: {
                storageBufferNeedsUpdate                = true;
                m_renderData.renderProxies[cmd.proxyId] = cmd.proxy;
                break;
            }
        }
    }

    if (storageBufferNeedsUpdate) { UpdateStorageBufferIfNeeded(); }
}

void MMeshInstanceManager::UpdateStorageBufferIfNeeded()
{
    auto renderSystem = GetEngine()->GetSystem<MRenderSystem>();

    if (m_renderData.instanceBuffer.GetSize() < m_renderData.renderProxies.size() * sizeof(MMeshInstanceRenderProxy))
    {
        m_renderData.instanceBuffer.ReallocMemory(m_renderData.renderProxies.size() * sizeof(MMeshInstanceRenderProxy));
    }

    m_renderData.instanceBuffer.UploadBuffer(
            renderSystem->GetDevice(),
            reinterpret_cast<MByte*>(m_renderData.renderProxies.data()),
            m_renderData.renderProxies.size() * sizeof(MMeshInstanceRenderProxy)
    );
}

void MMeshInstanceManager::OnMaterialChanged(MComponent* component)
{
    auto* meshComponent = component->template DynamicCast<MRenderMeshComponent>();
    if (!meshComponent) { return; }

    RemoveComponentFromGroup(meshComponent);

    auto material = meshComponent->GetMaterial();
    if (!material) { return; }

    AddComponentToGroup(meshComponent);
}

void MMeshInstanceManager::OnMeshChanged(MComponent* component)
{
    if (auto* meshComponent = component->template DynamicCast<MRenderMeshComponent>())
    {
        UpdateMeshInstance(meshComponent, CreateProxyFromComponent(meshComponent));
    }
}

void MMeshInstanceManager::OnRenderMeshChanged(MComponent* component)
{
    if (auto meshComponent = component->template DynamicCast<MRenderMeshComponent>())
    {
        UpdateMeshInstance(meshComponent, CreateProxyFromComponent(meshComponent));
    }
}

void MMeshInstanceManager::OnSceneComponentChanged(MComponent* component)
{
    if (auto* meshComponent = component->GetEntity()->GetComponent<MRenderMeshComponent>())
    {
        UpdateMeshInstance(meshComponent, CreateProxyFromComponent(meshComponent));
    }
}

void MMeshInstanceManager::OnRemoveComponent(MRenderMeshComponent* component) { RemoveComponentFromGroup(component); }

MMeshInstanceRenderProxy MMeshInstanceManager::CreateProxyFromComponent(MRenderMeshComponent* component)
{
    MMeshInstanceRenderProxy proxy;

    if (!component) { return proxy; }

    // Use component ID index as globally unique ID
    proxy.proxyId = static_cast<MMeshInstanceKey>(component->GetComponentID().nIdx);

    // Get visibility
    if (auto* pSceneComponent = component->GetEntity()->GetComponent<MSceneComponent>())
    {
        proxy.visible        = pSceneComponent->GetVisibleRecursively();
        proxy.worldTransform = pSceneComponent->GetWorldTransform();
    }
    else { proxy.visible = false; }

    return proxy;
}

void MMeshInstanceManager::AddComponentToGroup(MRenderMeshComponent* component)
{
    if (!component) { return; }

    auto material = component->GetMaterial();
    if (!material) { return; }

    auto pMaterialTemplate = material->GetTemplate();
    if (!pMaterialTemplate) { return; }

    const MMeshInstanceKey proxyId = static_cast<uint32_t>(component->GetComponentID().nIdx);

    // Record main thread mapping
    m_mainThreadData.componentToMaterialTemplate[proxyId] = pMaterialTemplate;

    // Check if batch group already exists in main thread cache
    auto                                 it = m_mainThreadData.materialTemplateToBatchGroup.find(pMaterialTemplate);
    std::shared_ptr<MMaterialBatchGroup> batchGroup;
    MMaterialInstanceKey                 materialInstanceKey = 0;

    if (it == m_mainThreadData.materialTemplateToBatchGroup.end())
    {
        // Create parameter set and get instance data name on main thread
        auto defaultPass = pMaterialTemplate->GetDefaultPass();
        if (!defaultPass) { return; }

        auto shaderProgram = defaultPass->GetShaderProgram();
        if (!shaderProgram) { return; }

        auto instanceDataName = shaderProgram->GetPropertyBlock().GetInstancingName();
        auto parameterSet     = pMaterialTemplate->CreateParameterSet(0);

        // Create batch group on main thread
        batchGroup = std::make_shared<MMaterialBatchGroup>(parameterSet, instanceDataName);
        batchGroup->SetMaterialTemplate(pMaterialTemplate);

        // Cache in main thread
        m_mainThreadData.materialTemplateToBatchGroup[pMaterialTemplate] = batchGroup;
        UpdateCommand cmd;
        cmd.type       = UpdateCommand::Type::AddGroup;
        cmd.batchGroup = batchGroup;
        m_pendingCommands.push_back(cmd);
    }
    else { batchGroup = it->second; }
    if (batchGroup) { materialInstanceKey = batchGroup->AddInstance(proxyId); }

    // Create add command
    UpdateCommand cmd;
    cmd.type             = UpdateCommand::Type::AddInstance;
    cmd.proxyId          = proxyId;
    cmd.proxy            = CreateProxyFromComponent(component);
    cmd.proxy.materialId = materialInstanceKey;
    cmd.batchGroup       = batchGroup;
    m_pendingCommands.push_back(cmd);
}

void MMeshInstanceManager::RemoveComponentFromGroup(MRenderMeshComponent* component)
{
    if (!component) { return; }

    const MMeshInstanceKey proxyId = static_cast<uint32_t>(component->GetComponentID().nIdx);

    // Remove from main thread mapping
    auto                   it = m_mainThreadData.componentToMaterialTemplate.find(proxyId);
    if (it == m_mainThreadData.componentToMaterialTemplate.end()) { return; }

    auto materialTemplate = it->second;
    m_mainThreadData.componentToMaterialTemplate.erase(it);

    auto findResult = m_mainThreadData.materialTemplateToBatchGroup.find(materialTemplate);
    auto batchGroup = findResult->second;

    if (batchGroup)
    {
        batchGroup->RemoveInstance(proxyId);
        if (batchGroup->IsEmpty())
        {
            UpdateCommand cmd;
            cmd.type       = UpdateCommand::Type::RemoveGroup;
            cmd.batchGroup = batchGroup;
            m_pendingCommands.push_back(cmd);
            m_mainThreadData.materialTemplateToBatchGroup.erase(findResult);
        }
    }

    // Create remove command
    UpdateCommand cmd;
    cmd.type    = UpdateCommand::Type::RemoveInstance;
    cmd.proxyId = proxyId;
    m_pendingCommands.push_back(cmd);
}

void MMeshInstanceManager::UpdateMeshInstance(MRenderMeshComponent* component, MMeshInstanceRenderProxy proxy)
{
    if (!component) { return; }

    // Check if component is already registered
    auto it = m_mainThreadData.componentToMaterialTemplate.find(proxy.proxyId);
    if (it == m_mainThreadData.componentToMaterialTemplate.end()) { return; }

    auto findResult = m_mainThreadData.materialTemplateToBatchGroup.find(it->second);
    if (findResult == m_mainThreadData.materialTemplateToBatchGroup.end()) { return; }
    auto batchGroup = findResult->second;
    if (batchGroup == nullptr) { return; }

    // Create update command
    UpdateCommand cmd;
    cmd.type             = UpdateCommand::Type::UpdateInstance;
    cmd.proxyId          = proxy.proxyId;
    cmd.proxy            = proxy;
    cmd.proxy.materialId = batchGroup->GetInstanceKey(proxy.proxyId);
    cmd.batchGroup       = batchGroup;
    m_pendingCommands.push_back(cmd);
}

void MMeshInstanceManager::Clean()
{
    m_mainThreadData.componentToMaterialTemplate.clear();
    m_mainThreadData.materialTemplateToBatchGroup.clear();

    m_renderData.renderProxies.clear();
    m_pendingCommands.clear();
    {
        std::lock_guard<std::mutex> lock(m_updateMutex);
        m_updateQueue.clear();
    }
}
