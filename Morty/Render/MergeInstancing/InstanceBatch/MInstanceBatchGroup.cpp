#include "MInstanceBatchGroup.h"

void MInstanceBatchGroup::UpdateMesh(MRenderableMeshComponent* pComponent)
{
    if (auto pInstance = FindMeshInstance(pComponent))
    {
        pInstance->pMesh = pComponent->GetMesh();
    }
}

void MInstanceBatchGroup::UpdateVisible(MRenderableMeshComponent* pComponent, bool bVisible)
{
    if (auto pInstance = FindMeshInstance(pComponent))
    {
        pInstance->bVisible = bVisible;
    }
}
