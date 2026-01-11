/**
 * @File         MFrameParamNode
 *
 * @Created      2025-01-11
 *
 * @Author       DoubleYe
 * @Brief        Render node that outputs per-frame ParameterSet (set = 1)
 *               containing camera and frame data
 **/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Render/RenderGraph/MRenderTaskNode.h"

namespace morty
{

class MMaterialTemplate;
class MShaderParameterSet;
class MFrameParameterSetAdapter;

REFL_RENDER_NODE_CLASS MFrameParamNode : public MRenderTaskNode
{
    MORTY_CLASS(MFrameParamNode)

public:
    void OnCreated() override;
    void OnDelete() override;
    void Execute(const MRenderInfo& info, IRenderCommand* primaryCommand) override;

    [[nodiscard]] std::shared_ptr<MShaderParameterSet> GetFrameParameterSet() const { return m_frameParameterSet; }

protected:
    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

private:
    void UpdateFrameParameters(const MRenderInfo& info);

    std::shared_ptr<MMaterialTemplate>         m_materialTemplate;
    std::shared_ptr<MShaderParameterSet>       m_frameParameterSet;
    std::shared_ptr<MFrameParameterSetAdapter> m_adapter;
};

}// namespace morty
