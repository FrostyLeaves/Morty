/**
 * @File         MSceneCullingNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

REFL_RENDER_NODE_CLASS MSceneCullingNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MSceneCullingNode)

public:
    void Execute(const MRenderInfo& info, IRenderCommand* primaryCommand) override;

protected:
    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty