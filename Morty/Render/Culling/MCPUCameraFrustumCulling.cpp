#include "MCPUCameraFrustumCulling.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "Batch/MMaterialBatchGroup.h"





void MCPUCameraFrustumCulling::Initialize(MEngine* pEngine)
{
	MCameraFrustumCulling::Initialize(pEngine);

	m_pFrustumFilter = std::make_shared<MCameraFrustumFilter>();
	m_pBoundingCulling = std::make_unique<MBoundingCulling>();
	m_pBoundingCulling->Initialize(pEngine);

	m_pBoundingCulling->AddFilter(m_pFrustumFilter);
}

void MCPUCameraFrustumCulling::Release()
{
	m_pBoundingCulling->Release();
	m_pBoundingCulling = nullptr;

	MCameraFrustumCulling::Release();
}

void MCPUCameraFrustumCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
	m_pFrustumFilter->m_cameraFrustum = m_cameraFrustum;

	m_pBoundingCulling->Culling(vInstanceGroup);
}

const MBuffer* MCPUCameraFrustumCulling::GetDrawIndirectBuffer()
{
	return m_pBoundingCulling->GetDrawIndirectBuffer();
}

const std::vector<MMaterialCullingGroup>& MCPUCameraFrustumCulling::GetCullingInstanceGroup() const
{
	return m_pBoundingCulling->GetCullingInstanceGroup();
}

bool MCameraFrustumFilter::Filter(const MMeshInstanceRenderProxy* instance) const
{
	const MBoundsAABB& bounds = instance->boundsWithTransform;
	if (instance->bCullEnable && MCameraFrustum::EOUTSIDE == m_cameraFrustum.ContainTest(bounds))
	{
		return false;
	}

	return true;
}