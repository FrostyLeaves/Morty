#include "MInstanceBatchGroup.h"

void MInstanceBatchGroup::UpdateVisible(MMeshInstanceKey key, bool bVisible)
{
    if (auto pInstance = FindMeshInstance(key))
    {
        pInstance->bVisible = bVisible;
    }
}
