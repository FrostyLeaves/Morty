#pragma once

#include "MInstanceCulling.h"

#include "Render/MBuffer.h"
#include "Render/MVertex.h"
#include "Shadow/MShadowMapUtil.h"

class IRenderableFilter;
class MInstanceBatchGroup;
class MORTY_API MCascadedShadowCulling
    : public MInstanceCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    MEngine* GetEngine() const { return m_pEngine; }

    void SetCamera(MEntity* pCameraEntity);
    void SetViewport(MViewport* pViewport) { m_pViewport = pViewport; }
    void SetDirectionalLight(MEntity* pDirectionalLight);
    void Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;
    void UploadBuffer(MIRenderCommand* pCommand) override;
    const MBuffer* GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override { return m_vCullingInstanceGroup; }
    const MCascadedArray<MCascadedShadowRenderData> GetCascadedRenderInfo() const { return m_vCascadedRenderData; }

protected:

    void CullingForDrawInstancing(const std::vector<MMaterialBatchGroup*>& vInstanceGroup, const MCascadedArray<std::unique_ptr<IRenderableFilter>>& vCascadedFilter);

private:

    MEngine* m_pEngine = nullptr;
    MViewport* m_pViewport = nullptr;
    MEntity* m_pCameraEntity = nullptr;
    MEntity* m_pDirectionalLight = nullptr;
    MBuffer m_drawIndirectBuffer;
    
	std::vector<MDrawIndexedIndirectData> m_vDrawIndirectData;

    //Shadow caster draw instancing.
    std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
    //Potential Shadow Caster
    MCascadedArray<MBoundsAABB> m_vCascadedPscBounds;
    MCascadedArray<MCascadedShadowRenderData> m_vCascadedRenderData;
};
