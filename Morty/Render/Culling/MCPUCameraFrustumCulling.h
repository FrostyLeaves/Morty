#pragma once

#include "MBoundingCulling.h"
#include "MInstanceCulling.h"

MORTY_SPACE_BEGIN

class MInstanceBatchGroup;

class MORTY_API MCameraFrustumFilter : public IMeshInstanceFilter
{
public:

    bool Filter(const MMeshInstanceRenderProxy* instance) const override;

	MCameraFrustum m_cameraFrustum;
};


class MORTY_API MCPUCameraFrustumCulling
    : public MCameraFrustumCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;


    void Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;
    void UploadBuffer(MIRenderCommand* pCommand) override;
    const MBuffer* GetDrawIndirectBuffer() override;
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override;

private:
    std::shared_ptr<MCameraFrustumFilter> m_pFrustumFilter = nullptr;
    std::unique_ptr<MBoundingCulling> m_pBoundingCulling = nullptr;
};

MORTY_SPACE_END