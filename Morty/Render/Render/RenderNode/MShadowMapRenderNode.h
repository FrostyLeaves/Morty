/**
 * @File         MShadowMapRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"

namespace morty
{

class MTaskNode;
class IRenderable;
class IRenderCommand;

REFL_RENDER_NODE_CLASS MShadowMapRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MShadowMapRenderNode)

public:
    void Execute(const MRenderInfo& info, IRenderCommand* primaryCommand) override;

protected:
    void                               OnCreated() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty