#include "MInstanceCulling.h"
#include "MergeInstancing/InstanceBatch/MInstanceBatchGroup.h"

void CameraFrustumCulling::UpdateCameraFrustum(Matrix4 cameraInvProj)
{
	m_cameraFrustum.UpdateFromCameraInvProj(cameraInvProj);
}

bool CameraFrustumCulling::Filter(const MMeshInstanceRenderProxy* instance) const
{
	if (!instance->bCullEnable)
	{
		return true;
	}

	const MBoundsAABB& bounds = instance->boundsWithTransform;
	if (MCameraFrustum::EOUTSIDE == m_cameraFrustum.ContainTest(bounds))
	{
		return false;
	}

	return true;
}


bool MMaterialTypeFilter::Filter(const std::shared_ptr<MMaterial>& material) const
{
	return material->GetMaterialType() == m_eMaterialType;
}