/**
 * @File         MRenderGraph
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "RHI/MRenderPass.h"
#include "TaskGraph/MTaskNodeOutput.h"

#include "Render/MRenderInfo.h"
#include "Render/MeshRender/MIndirectIndexRenderable.h"
#include "Render/RenderGraph/MRenderTargetManager.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderGraphSetting;
class IPropertyBlockAdapter;
class MRenderTaskTarget;
class MRenderTargetManager;
class MORTY_API MRenderGraph : public MTaskGraph
{
    MORTY_CLASS(MRenderGraph)
public:
    MRenderGraph() = default;

    explicit MRenderGraph(MEngine* pEngine);

    ~MRenderGraph() override;

    [[nodiscard]] MRenderTaskNode* RegisterTaskNode(const MStringId& strTaskNodeName, const MString& strTaskNodeType);
    [[nodiscard]] MRenderTaskNode* FindTaskNode(const MStringId& strTaskNodeName) const;
    [[nodiscard]] MRenderTargetManager*                GetRenderTargetManager() const { return m_renderTargetManager; }
    [[nodiscard]] std::shared_ptr<MRenderGraphSetting> GetRenderGraphSetting() const { return m_renderGraphSetting; }
    MRenderTaskTarget*                                 FindRenderTaskTarget(const MStringId& name);

    void SetFrameProperty(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter) { m_framePropertyAdapter = pAdapter; }

    [[nodiscard]] const std::shared_ptr<IPropertyBlockAdapter>& GetFrameProperty() const
    {
        return m_framePropertyAdapter;
    }

    void SetCameraCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_cameraCullingResult = pAdapter; }

    [[nodiscard]] const std::shared_ptr<MInstanceCulling>& GetCameraCullingResult() const
    {
        return m_cameraCullingResult;
    }

    void SetShadowCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_shadowCullingResult = pAdapter; }

    [[nodiscard]] const std::shared_ptr<MInstanceCulling>& GetShadowCullingResult() const
    {
        return m_shadowCullingResult;
    }

    void SetVoxelizerCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter)
    {
        m_voxelizerCullingResult = pAdapter;
    }

    [[nodiscard]] const std::shared_ptr<MInstanceCulling>& GetVoxelizerCullingResult() const
    {
        return m_voxelizerCullingResult;
    }

    void Resize(const Vector2i& size);
    void Clean();

private:
    Vector2i                                    m_size = {0, 0};

    MEngine*                                    m_engine              = nullptr;
    MRenderTargetManager*                       m_renderTargetManager = nullptr;
    std::shared_ptr<MRenderGraphSetting>        m_renderGraphSetting  = nullptr;

    std::shared_ptr<IPropertyBlockAdapter>      m_framePropertyAdapter   = nullptr;
    std::shared_ptr<MInstanceCulling>           m_cameraCullingResult    = nullptr;
    std::shared_ptr<MInstanceCulling>           m_shadowCullingResult    = nullptr;
    std::shared_ptr<MInstanceCulling>           m_voxelizerCullingResult = nullptr;

    std::map<const MStringId, MRenderTaskNode*> m_taskNodeTable;
};

}// namespace morty