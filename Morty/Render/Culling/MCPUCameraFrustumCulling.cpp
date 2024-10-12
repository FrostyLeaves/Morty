#include "MCPUCameraFrustumCulling.h"

#include "Batch/MMaterialBatchGroup.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MEntity.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"

using namespace morty;

void MCPUCameraFrustumCulling::Initialize(MEngine* pEngine)
{
    MCameraFrustumCulling::Initialize(pEngine);

    m_frustumFilter   = std::make_shared<MCameraFrustumFilter>();
    m_boundingCulling = std::make_unique<MBoundingCulling>();
    m_boundingCulling->Initialize(pEngine);

    m_boundingCulling->AddFilter(m_frustumFilter);
}

void MCPUCameraFrustumCulling::Release()
{
    m_boundingCulling->Release();
    m_boundingCulling = nullptr;

    MCameraFrustumCulling::Release();
}

void MCPUCameraFrustumCulling::Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup)
{
    m_frustumFilter->m_cameraFrustum = m_cameraFrustum;

    m_boundingCulling->Culling(vInstanceGroup);
}

void MCPUCameraFrustumCulling::UploadBuffer(MIRenderCommand* pCommand) { m_boundingCulling->UploadBuffer(pCommand); }

const MBuffer* MCPUCameraFrustumCulling::GetDrawIndirectBuffer() { return m_boundingCulling->GetDrawIndirectBuffer(); }

const std::vector<MMaterialCullingGroup>& MCPUCameraFrustumCulling::GetCullingInstanceGroup() const
{
    return m_boundingCulling->GetCullingInstanceGroup();
}

bool MCameraFrustumFilter::Filter(const MMeshInstanceRenderProxy* instance) const
{
    const MBoundsAABB& bounds = instance->boundsWithTransform;
    if (instance->bCullEnable && MCameraFrustum::EOUTSIDE == m_cameraFrustum.ContainTest(bounds)) { return false; }

    return true;
}