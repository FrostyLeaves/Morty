#ifndef _M_SCENE_GPU_CULLING_H_
#define _M_SCENE_GPU_CULLING_H_

#include "MInstanceCulling.h"

#include "Render/MBuffer.h"
#include "Render/MVertex.h"

class MComputeDispatcher;
class MInstanceBatchGroup;
class MORTY_API MSceneGPUCulling
    : public MInstanceCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    MEngine* GetEngine() const { return m_pEngine; }

    void SetCommand(MIRenderCommand* pCommand) { m_pCommand = pCommand; }
    void SetCameraFrustum(MCameraFrustum cameraFrustum) { m_cameraFrustum = cameraFrustum; }
    void SetCameraPosition(Vector3 v3CameraPosition) { m_v3CameraPosition = v3CameraPosition; }
    void AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter);

    void UpdateCullingCamera();
    void Culling(const std::vector<MRenderableMaterialGroup*>& vInstanceGroup) override;
    const MBuffer* GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override { return m_vCullingInstanceGroup; }
private:

    Vector3 m_v3CameraPosition;
    MCameraFrustum m_cameraFrustum;

    MEngine* m_pEngine = nullptr;
    MIRenderCommand* m_pCommand = nullptr;
    MComputeDispatcher* m_pCullingComputeDispatcher = nullptr;
    MBuffer m_cullingInputBuffer;
    MBuffer m_drawIndirectBuffer;
    MBuffer m_cullingOutputBuffer;
    std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
    std::vector<std::shared_ptr<IMeshInstanceFilter>> m_vFilter;

};



#endif