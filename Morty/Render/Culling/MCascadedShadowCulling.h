#pragma once

#include "MInstanceCulling.h"

#include "Basic/MBuffer.h"
#include "Mesh/MVertex.h"
#include "Shadow/MShadowMapUtil.h"

namespace morty
{

class IRenderableFilter;
class MInstanceBatchGroup;
class MORTY_API MCascadedShadowCulling : public MInstanceCulling
{
public:
    void                                      Initialize(MEngine* pEngine) override;

    void                                      Release() override;

    MEngine*                                  GetEngine() const { return m_engine; }

    void                                      SetCamera(MEntity* pCameraEntity);

    void                                      SetViewport(MViewport* pViewport) { m_viewport = pViewport; }

    void                                      SetDirectionalLight(MEntity* pDirectionalLight);

    void                                      Culling(const std::vector<MMaterialBatchGroup*>& vInstanceGroup) override;

    void                                      UploadBuffer(MIRenderCommand* pCommand) override;

    const MBuffer*                            GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }

    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override
    {
        return m_cullingInstanceGroup;
    }

    const MCascadedArray<MCascadedShadowRenderData> GetCascadedRenderInfo() const { return m_cascadedRenderData; }

protected:
    void CullingForDrawInstancing(
            const std::vector<MMaterialBatchGroup*>&                  vInstanceGroup,
            const MCascadedArray<std::unique_ptr<IRenderableFilter>>& vCascadedFilter
    );

private:
    MEngine*                                  m_engine           = nullptr;
    MViewport*                                m_viewport         = nullptr;
    MEntity*                                  m_cameraEntity     = nullptr;
    MEntity*                                  m_directionalLight = nullptr;
    MBuffer                                   m_drawIndirectBuffer;

    std::vector<MDrawIndexedIndirectData>     m_drawIndirectData;

    //Shadow caster draw instancing.
    std::vector<MMaterialCullingGroup>        m_cullingInstanceGroup;
    //Potential Shadow Caster
    MCascadedArray<MBoundsAABB>               m_cascadedPscBounds;
    MCascadedArray<MCascadedShadowRenderData> m_cascadedRenderData;
};

}// namespace morty