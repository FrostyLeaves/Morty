/**
 * @File         MColorGradingRenderWork
 * 
 * @Created      2024-02-19 21:01:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MBasicPostProcessRenderWork.h"
#include "MSinglePassRenderWork.h"

#include "Basic/MCameraFrustum.h"
#include "MRenderWork.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"

namespace morty
{

class MORTY_API MColorGradingRenderWork : public MBasicPostProcessRenderWork
{
    MORTY_CLASS(MColorGradingRenderWork)

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