/**
 * @File         MToneMappingRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBasicPostProcessRenderNode.h"
#include "MSinglePassRenderNode.h"

#include "Basic/MCameraFrustum.h"
#include "MRenderNode.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"

namespace morty
{

class MORTY_API MToneMappingRenderNode : public MBasicPostProcessRenderNode
{
    MORTY_CLASS(MToneMappingRenderNode)

    static const MStringId     ToneMappingResult;

    std::shared_ptr<MMaterial> CreateMaterial() override;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty