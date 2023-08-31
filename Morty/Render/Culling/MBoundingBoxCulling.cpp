#include "MBoundingBoxCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Batch/MMaterialBatchGroup.h"





void MBoundingBoxCulling::Initialize(MEngine* pEngine)
{
	MBoundingCulling::Initialize(pEngine);

	m_pBoundsFilter = std::make_shared<MBoundingBoxFilter>();

	AddFilter(m_pBoundsFilter);
}

void MBoundingBoxCulling::Release()
{
	MBoundingCulling::Release();
}

void MBoundingBoxCulling::SetBounds(const MBoundsAABB& bounds)
{
	m_pBoundsFilter->m_bounds = bounds;
}

bool MBoundingBoxFilter::Filter(const MMeshInstanceRenderProxy* instance) const
{
	const MBoundsAABB& bounds = instance->boundsWithTransform;

	return m_bounds.IsIntersect(bounds);
}