/**
 * @File         MGBufferRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSinglePassRenderWork.h"

#include "Basic/MCameraFrustum.h"
#include "MRenderWork.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"

namespace morty
{

class MORTY_API MGBufferRenderWork : public ISinglePassRenderWork
{
    MORTY_CLASS(MGBufferRenderWork)

    static const MStringId GBufferAlbedoMetallic;
    static const MStringId GBufferNormalRoughness;
    static const MStringId GBufferPositionAmbientOcc;
    static const MStringId GBufferDepthBufferOutput;

public:
    void                             Render(const MRenderInfo& info) override;
    void                             Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    std::shared_ptr<IGBufferAdapter> CreateGBuffer();


protected:
    void                               BindTarget() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty