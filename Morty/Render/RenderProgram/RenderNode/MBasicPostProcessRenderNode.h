/**
 * @File         MBasicPostProcessRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

class MORTY_API MBasicPostProcessRenderNode : public ISinglePassRenderNode
{
    MORTY_INTERFACE(MBasicPostProcessRenderNode)

    void                               Initialize(MEngine* pEngine) override;

    void                               Release() override;

    void                               Render(const MRenderInfo& info) override;

    virtual std::shared_ptr<MMaterial> CreateMaterial() = 0;

protected:
    void                       BindTarget() override;

    std::shared_ptr<MMaterial> m_material = nullptr;
};

}// namespace morty