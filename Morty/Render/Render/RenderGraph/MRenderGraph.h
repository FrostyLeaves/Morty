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
#include "MRenderTargetBindingWalker.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/MeshRender/MIndirectIndexRenderable.h"
#include "TaskGraph/MTaskGraph.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderGraphSetting;
class IPropertyBlockAdapter;
class MORTY_API MRenderGraph : public MTaskGraph
{
    MORTY_CLASS(MRenderGraph)
public:
    MRenderGraph() = default;

    explicit MRenderGraph(MEngine* pEngine);

    ~MRenderGraph() override;

    [[nodiscard]] bool             AddNode(const MStringId& strNodeName, MTaskNode* pGraphNode) override;
    [[nodiscard]] MEngine*         GetEngine() const { return m_engine; }
    [[nodiscard]] MRenderTaskNode* FindRenderNode(size_t renderNodeId) const;
    [[nodiscard]] std::shared_ptr<MRenderGraphSetting> GetRenderGraphSetting() const { return m_renderGraphSetting; }

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

    void                      OnPreCompile() override;
    void                      OnPostCompile() override;
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
    void                      Deserialize(const void* pBufferPointer) override;

    void                      Resize(const Vector2i& size);
    void                      Clean();

private:
    Vector2i                                    m_size = {0, 0};

    MEngine*                                    m_engine             = nullptr;
    std::shared_ptr<MRenderGraphSetting>        m_renderGraphSetting = nullptr;

    std::shared_ptr<IPropertyBlockAdapter>      m_framePropertyAdapter   = nullptr;
    std::shared_ptr<MInstanceCulling>           m_cameraCullingResult    = nullptr;
    std::shared_ptr<MInstanceCulling>           m_shadowCullingResult    = nullptr;
    std::shared_ptr<MInstanceCulling>           m_voxelizerCullingResult = nullptr;
    std::unique_ptr<MRenderTargetBindingWalker> m_renderTargetBinding    = nullptr;
    std::map<const MStringId, MRenderTaskNode*> m_taskNodeTable;
};

}// namespace morty