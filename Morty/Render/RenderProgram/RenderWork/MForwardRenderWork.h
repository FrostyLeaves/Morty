/**
 * @File         MForwardRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MDeferredLightingRenderWork.h"
#include "MSinglePassRenderWork.h"

#include "MRenderWork.h"
#include "RenderProgram/MRenderInfo.h"

namespace morty
{

class MORTY_API MForwardRenderWork : public ISinglePassRenderWork
{
    MORTY_CLASS(MForwardRenderWork)
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