#include "MMaterialBatchGroup.h"
#include "Component/MRenderMeshComponent.h"
#include "Engine/MEngine.h"
#include "Material/MMaterialTemplate.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Abstract/MIDevice.h"
#include "System/MRenderSystem.h"


using namespace morty;

MMaterialBatchGroup::MMaterialBatchGroup(
        const std::shared_ptr<MShaderParameterSet>& parameterSet,
        const MStringId&                            name
)
    : m_parameterSet(parameterSet)
{
    if (auto storage = parameterSet->FindStorageParam(name)) { m_instanceDataSize = storage->var.GetSize(); }

    m_materialData = MBuffer::CreateStorageBuffer("material batch group buffer");
}

MMaterialInstanceKey MMaterialBatchGroup::AddInstance(MMeshInstanceKey proxyId)
{
    auto id                  = m_idPool.AllocateID();
    m_instanceTable[proxyId] = id;
    return id;
}

void MMaterialBatchGroup::RemoveInstance(MMeshInstanceKey proxyId)
{
    m_instanceTable.erase(proxyId);
    m_idPool.FreeID(proxyId);
}

MMaterialInstanceKey MMaterialBatchGroup::GetInstanceKey(MMeshInstanceKey proxyId) const
{
    auto it = m_instanceTable.find(proxyId);
    if (it != m_instanceTable.end()) { return it->second; }

    return MGlobal::M_INVALID_INDEX;
}
