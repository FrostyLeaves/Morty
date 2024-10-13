#pragma once

#include "Basic/MBuffer.h"
#include "MInstanceCulling.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"

namespace morty
{

class MInstanceBatchGroup;
class MORTY_API MBoundingCulling : public MInstanceCulling
{
public:
    void                                      Initialize(MEngine* pEngine) override;

    void                                      Release() override;

    MEngine*                                  GetEngine() const { return m_engine; }

    void                                      AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter);

    void                                      Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;

    void                                      UploadBuffer(MIRenderCommand* pCommand) override;

    const MBuffer*                            GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }

    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override
    {
        return m_cullingInstanceGroup;
    }

private:
    MEngine*                                          m_engine = nullptr;
    MBuffer                                           m_drawIndirectBuffer;

    std::vector<MDrawIndexedIndirectData>             m_drawIndirectData;
    std::vector<MMaterialCullingGroup>                m_cullingInstanceGroup;
    std::vector<std::shared_ptr<IMeshInstanceFilter>> m_filter;
};

}// namespace morty