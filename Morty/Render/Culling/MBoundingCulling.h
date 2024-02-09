#pragma once

#include "MInstanceCulling.h"

#include "Render/MBuffer.h"
#include "Render/MIDevice.h"
#include "Render/MVertex.h"

class MInstanceBatchGroup;

class MORTY_API MBoundingCulling
    : public MInstanceCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    MEngine* GetEngine() const { return m_pEngine; }

    void AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter);

    void Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;
    void UploadBuffer(MIRenderCommand* pCommand) override;
    const MBuffer* GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override { return m_vCullingInstanceGroup; }
private:

    MEngine* m_pEngine = nullptr;
    MBuffer m_drawIndirectBuffer;
    
	std::vector<MDrawIndexedIndirectData> m_vDrawIndirectData;
    std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
    std::vector<std::shared_ptr<IMeshInstanceFilter>> m_vFilter;

};
