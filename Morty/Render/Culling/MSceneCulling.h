#ifndef _M_SCENE_CULLING_H_
#define _M_SCENE_CULLING_H_

#include "MInstanceCulling.h"

#include "Render/MBuffer.h"
#include "Render/MVertex.h"

class MInstanceBatchGroup;

class MORTY_API MSceneCulling
    : public MInstanceCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    MEngine* GetEngine() const { return m_pEngine; }

    void AddFilter(std::shared_ptr<IMeshInstanceFilter> pFilter);

    void Culling(const std::vector<MRenderableMaterialGroup*>& vInstanceGroup) override;
    const MBuffer* GetDrawIndirectBuffer() override { return &m_drawIndirectBuffer; }
    const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const override { return m_vCullingInstanceGroup; }
private:

    MEngine* m_pEngine = nullptr;
    MBuffer m_drawIndirectBuffer;
    std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
    std::vector<std::shared_ptr<IMeshInstanceFilter>> m_vFilter;

};



#endif