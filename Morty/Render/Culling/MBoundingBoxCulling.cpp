#include "MBoundingBoxCulling.h"

#include "Batch/MMaterialBatchGroup.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"

using namespace morty;

void MBoundingBoxCulling::Initialize(MEngine* pEngine)
{
    MBoundingCulling::Initialize(pEngine);

    m_boundsFilter = std::make_shared<MBoundingBoxFilter>();

    AddFilter(m_boundsFilter);
}

void MBoundingBoxCulling::Release() { MBoundingCulling::Release(); }

void MBoundingBoxCulling::SetBounds(const MBoundsAABB& bounds) { m_boundsFilter->m_bounds = bounds; }

bool MBoundingBoxFilter::Filter(const MMeshInstanceRenderProxy* instance) const
{
    const MBoundsAABB& bounds = instance->boundsWithTransform;

    return m_bounds.IsIntersect(bounds);
}