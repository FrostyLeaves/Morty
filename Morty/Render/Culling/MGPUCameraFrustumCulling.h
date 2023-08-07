#pragma once

#include "MInstanceCulling.h"

#include "Render/MBuffer.h"
#include "Render/MVertex.h"

class MComputeDispatcher;
class MInstanceBatchGroup;
class MORTY_API MGPUCameraFrustumCulling
    : public MCameraFrustumCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    MEngine* GetEngine() const { return m_pEngine; }
    
    void AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter);

    void UpdateCullingCamera();
    void Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;
    const MBuffer* GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override { return m_vCullingInstanceGroup; }
private:

    MEngine* m_pEngine = nullptr;
    MComputeDispatcher* m_pCullingComputeDispatcher = nullptr;
    MBuffer m_cullingInputBuffer;
    MBuffer m_drawIndirectBuffer;
    MBuffer m_cullingOutputBuffer;
    std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
    std::vector<std::shared_ptr<IMeshInstanceFilter>> m_vFilter;

};
