/**
 * @File         MBlurRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBasicPostProcessRenderNode.h"

namespace morty
{

class MORTY_API MBlurRenderNode : public MBasicPostProcessRenderNode
{
    MORTY_CLASS(MBlurRenderNode)

    void                       InitDirection(bool bVertical) { m_vertical = bVertical; }

    std::shared_ptr<MMaterial> CreateMaterial() override;

    void                       RenderSetup(const MRenderInfo& info) override;

    void                       RegisterSetting() override;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

    bool                               m_vertical = false;
};

}// namespace morty