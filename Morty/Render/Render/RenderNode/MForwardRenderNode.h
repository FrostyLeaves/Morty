/**
 * @File         MForwardRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MDeferredLightingRenderNode.h"
#include "RenderProgram/RenderGraph/MSinglePassRenderNode.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderCommon.h"

namespace morty
{

class MORTY_API MForwardRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MForwardRenderNode)
    static const MStringId BackBufferOutput;
    static const MStringId DepthBufferOutput;

public:
    void Render(const MRenderInfo& info) override;

    void Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);


protected:
    void                               BindTarget() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty