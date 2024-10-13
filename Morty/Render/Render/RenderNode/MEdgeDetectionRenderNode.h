/**
 * @File         MEdgeDetectionRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBasicPostProcessRenderNode.h"
#include "RenderProgram/RenderGraph/MSinglePassRenderNode.h"

#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderCommon.h"

namespace morty
{

class MORTY_API MEdgeDetectionRenderNode : public MBasicPostProcessRenderNode
{
    MORTY_CLASS(MEdgeDetectionRenderNode)

    static const MStringId     EdgeDetectionResult;

    std::shared_ptr<MMaterial> CreateMaterial() override;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty