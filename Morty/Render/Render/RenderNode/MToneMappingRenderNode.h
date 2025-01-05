/**
 * @File         MToneMappingRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "MBasicPostProcessRenderNode.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

REFL_RENDER_NODE_CLASS MToneMappingRenderNode : public MBasicPostProcessRenderNode
{
    MORTY_CLASS(MToneMappingRenderNode)

    MMaterialTemplatePtr CreateMaterial() override;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty