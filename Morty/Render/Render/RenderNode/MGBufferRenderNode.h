/**
 * @File         MGBufferRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

REFL_RENDER_NODE_CLASS MGBufferRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MGBufferRenderNode)

    static const MStringId GBufferAlbedoMetallic;
    static const MStringId GBufferNormalRoughness;
    static const MStringId GBufferPositionAmbientOcc;
    static const MStringId GBufferDepthBufferOutput;

public:
    void                             Render(const MRenderInfo& info) override;
    void                             Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    std::shared_ptr<IGBufferAdapter> CreateGBuffer();


protected:
    void                               BindInOutTexture() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty