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
    [[nodiscard]] MRenderTaskNode* FindRenderNode(const MStringId& nodeName) const;
    [[nodiscard]] std::shared_ptr<MRenderGraphSetting> GetRenderGraphSetting() const { return m_renderGraphSetting; }

    void                                               SetFinalOutput(size_t nNodeIdx, size_t nSlotIdx);
    [[nodiscard]] MTexturePtr                          GetFinalOutput() const;
    [[nodiscard]] size_t                               GetFinalOutputNodeIdx() const { return m_finalOutputNodeId; }
    [[nodiscard]] size_t                               GetFinalOutputSlotIdx() const { return m_finalOutputSlotId; }

    void                      OnPreCompile() override;
    void                      OnPostCompile() override;
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
    void                      Deserialize(const void* pBufferPointer) override;

    void                      Resize(const Vector2i& size);
    [[nodiscard]] Vector2i    GetSize() const { return m_size; }

private:
    Vector2i                                    m_size = {0, 0};

    MEngine*                                    m_engine             = nullptr;
    std::shared_ptr<MRenderGraphSetting>        m_renderGraphSetting = nullptr;
    std::unique_ptr<MRenderTargetBindingWalker> m_renderTargetBinding    = nullptr;
    std::map<const MStringId, MRenderTaskNode*> m_taskNodeTable          = {};
    size_t                                      m_finalOutputNodeId      = 0;
    size_t                                      m_finalOutputSlotId      = 0;
};

}// namespace morty