#ifndef _M_CASCADED_SHADOW_CULLING_H_
#define _M_CASCADED_SHADOW_CULLING_H_

#include "MInstanceCulling.h"

#include "Render/MBuffer.h"
#include "Render/MVertex.h"

class MInstanceBatchGroup;
class MORTY_API MCascadedShadowCulling
    : public MInstanceCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    MEngine* GetEngine() const { return m_pEngine; }

    void SetViewport(MViewport* pViewport);
    void SetCamera(MEntity* pCameraEntity);
    void SetDirectionalLight(MEntity* pDirectionalLight);
    void Culling(const std::vector<MRenderableMaterialGroup*>& vInstanceGroup) override;
    const MBuffer* GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override { return m_vCullingInstanceGroup; }
    std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> GetCascadedRenderData() const { return m_vCascadedRenderData; }
private:

    MEngine* m_pEngine = nullptr;
    MViewport* m_pViewport = nullptr;
    MEntity* m_pCameraEntity = nullptr;
    MEntity* m_pDirectionalLight = nullptr;
    MBuffer m_drawIndirectBuffer;
    std::array<MBoundsAABB, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> m_vCascadedPcsBounds;
    std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> m_vCascadedRenderData;
    std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;

};



#endif