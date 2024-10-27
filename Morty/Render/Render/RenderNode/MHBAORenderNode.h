/**
 * @File         MHBAORenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBasicPostProcessRenderNode.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"

namespace morty
{

class MORTY_API MHBAORenderNode : public MBasicPostProcessRenderNode
{
    MORTY_CLASS(MHBAORenderNode)

    void                       Release() override;

    std::shared_ptr<MMaterial> CreateMaterial() override;

    void                       RenderSetup(const MRenderInfo& info) override;

public:
    REFL_RENDER_NODE_PROPERTY float HbaoRadius       = 2.0f;
    REFL_RENDER_NODE_PROPERTY float HbaoNearestScale = 1.0f;
    REFL_RENDER_NODE_PROPERTY float HbaoOtherScale   = 1.0f;
    REFL_RENDER_NODE_PROPERTY float HbaoNDotVBias    = 0.2f;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty