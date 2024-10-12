/**
 * @File         MHBAORenderWork
 * 
 * @Created      2021-08-16 10:37:01
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

class MORTY_API MHBAORenderWork : public MBasicPostProcessRenderWork
{
    MORTY_CLASS(MHBAORenderWork)

    static const MStringId     HBAOOutput;

    void                       Release() override;

    std::shared_ptr<MMaterial> CreateMaterial() override;

    void                       RenderSetup(const MRenderInfo& info) override;

    void                       RegisterSetting() override;

protected:
    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty