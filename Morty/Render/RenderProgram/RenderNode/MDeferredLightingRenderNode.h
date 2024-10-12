/**
 * @File         MDeferredLightingRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MGBufferRenderNode.h"
#include "MSinglePassRenderNode.h"

#include "MRenderNode.h"
#include "MShadowMapRenderNode.h"
#include "RenderProgram/MRenderInfo.h"

namespace morty
{

class MORTY_API MDeferredLightingRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MDeferredLightingRenderNode)
    static const MStringId DeferredLightingOutput;

    void                   Initialize(MEngine* pEngine) override;

    void                   Release() override;

    void                   Render(const MRenderInfo& info) override;

protected:
    void                               BindTarget() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;


private:
    std::shared_ptr<MMaterial> m_lightningMaterial = nullptr;
};

}// namespace morty