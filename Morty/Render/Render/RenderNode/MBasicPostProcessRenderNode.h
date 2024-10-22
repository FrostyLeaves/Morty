/**
 * @File         MBasicPostProcessRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

class MORTY_API MBasicPostProcessRenderNode : public ISinglePassRenderNode
{
    MORTY_INTERFACE(MBasicPostProcessRenderNode)

    void                               OnCreated() override;

    void                               Release() override;

    void                               Render(const MRenderInfo& info) override;

    virtual std::shared_ptr<MMaterial> CreateMaterial() = 0;

protected:
    void                       BindRenderTarget() override;

    std::shared_ptr<MMaterial> m_material = nullptr;
};

}// namespace morty