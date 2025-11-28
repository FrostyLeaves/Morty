#include "MMaterialBatchGroup.h"
#include "Component/MRenderMeshComponent.h"
#include "Engine/MEngine.h"
#include "Material/MMaterialTemplate.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Abstract/MIDevice.h"
#include "System/MRenderSystem.h"


using namespace morty;

MMaterialInstanceKey MMaterialBatchGroup::AddInstance(MMeshInstanceKey proxyId)
{
    auto id                  = m_idPool.AllocateID();
    m_materialTable[proxyId] = id;
    return id;
}

void                 MMaterialBatchGroup::RemoveInstance(MMeshInstanceKey proxyId) { m_materialTable.erase(proxyId); }

MMaterialInstanceKey MMaterialBatchGroup::GetInstanceKey(MMeshInstanceKey proxyId) const
{
    auto it = m_materialTable.find(proxyId);
    if (it != m_materialTable.end()) { return it->second; }

    return MGlobal::M_INVALID_INDEX;
}
