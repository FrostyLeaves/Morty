/**
 * @File         MColorGradingRenderNode
 * 
 * @Created      2024-02-19 21:01:01
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

class MORTY_API MColorGradingRenderNode : public MBasicPostProcessRenderNode
{
    MORTY_CLASS(MColorGradingRenderNode)

    static const MStringId     ColorGradingOutput;

    void                       Release() override;

    std::shared_ptr<MMaterial> CreateMaterial() override;

    void                       RenderSetup(const MRenderInfo& info) override;

    void                       RegisterSetting() override;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty