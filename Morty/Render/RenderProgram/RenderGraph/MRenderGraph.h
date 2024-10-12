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

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/MeshRender/MIndirectIndexRenderable.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"
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

    MRenderGraph(MEngine* pEngine);

    ~MRenderGraph() override;

    template<typename TYPE> MRenderTaskNode* RegisterTaskNode(const MStringId& strTaskNodeName);

    template<typename TYPE> TYPE*            FindTaskNode(const MStringId& strTaskNodeName) const;

    MRenderTargetManager*                    GetRenderTargetManager() const { return m_renderTargetManager; }

    std::shared_ptr<MRenderGraphSetting>     GetRenderGraphSetting() const { return m_renderGraphSetting; }

    MRenderTaskTarget*                       FindRenderTaskTarget(const MStringId& name);

    void SetFrameProperty(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter) { m_framePropertyAdapter = pAdapter; }

    const std::shared_ptr<IPropertyBlockAdapter>& GetFrameProperty() const { return m_framePropertyAdapter; }

    void SetCameraCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_cameraCullingResult = pAdapter; }

    const std::shared_ptr<MInstanceCulling>& GetCameraCullingResult() const { return m_cameraCullingResult; }

    void SetShadowCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_shadowCullingResult = pAdapter; }

    const std::shared_ptr<MInstanceCulling>& GetShadowCullingResult() const { return m_shadowCullingResult; }

    void SetVoxelizerCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter)
    {
        m_voxelizerCullingResult = pAdapter;
    }

    const std::shared_ptr<MInstanceCulling>& GetVoxelizerCullingResult() const { return m_voxelizerCullingResult; }

    void                                     Resize(const Vector2i& size);

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

template<typename TYPE> MRenderTaskNode* MRenderGraph::RegisterTaskNode(const MStringId& strTaskNodeName)
{
    const auto findResult = m_taskNodeTable.find(strTaskNodeName);
    if (findResult != m_taskNodeTable.end()) { return findResult->second; }

    auto pRenderWork = AddNode<TYPE>(strTaskNodeName);

    m_taskNodeTable[strTaskNodeName] = pRenderWork;

    return pRenderWork;
}

template<typename TYPE> TYPE* MRenderGraph::FindTaskNode(const MStringId& strTaskNodeName) const
{
    const auto findResult = m_taskNodeTable.find(strTaskNodeName);
    if (findResult != m_taskNodeTable.end()) { return findResult->second->DynamicCast<TYPE>(); }

    return nullptr;
}

}// namespace morty