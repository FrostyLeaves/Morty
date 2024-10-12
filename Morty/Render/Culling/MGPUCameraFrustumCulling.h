#pragma once

#include "MInstanceCulling.h"

#include "Basic/MBuffer.h"
#include "Mesh/MVertex.h"

namespace morty
{

class MComputeDispatcher;
class MInstanceBatchGroup;
class MORTY_API MGPUCameraFrustumCulling : public MCameraFrustumCulling
{
public:
    void                                      Initialize(MEngine* pEngine) override;

    void                                      Release() override;

    MEngine*                                  GetEngine() const { return m_engine; }

    void                                      UpdateCullingCamera();

    void                                      Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;

    const MBuffer*                            GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }

    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override
    {
        return m_cullingInstanceGroup;
    }

private:
    MEngine*                           m_engine                   = nullptr;
    MComputeDispatcher*                m_cullingComputeDispatcher = nullptr;
    MBuffer                            m_cullingInputBuffer;
    MBuffer                            m_drawIndirectBuffer;
    MBuffer                            m_cullingOutputBuffer;
    std::vector<MMaterialCullingGroup> m_cullingInstanceGroup;
};

}// namespace morty