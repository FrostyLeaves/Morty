/**
 * @File         MShadowMapRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"

namespace morty
{

class MTaskNode;
class IRenderable;
class MIRenderCommand;

REFL_RENDER_NODE_CLASS MShadowMapRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MShadowMapRenderNode)
    static const MStringId ShadowMapBufferOutput;

public:
    void Render(const MRenderInfo& info) override;
    void Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    [[nodiscard]] std::shared_ptr<IGetTextureAdapter> GetShadowMap() const;

    std::shared_ptr<IShaderPropertyUpdateDecorator>   GetFramePropertyDecorator() override;

protected:
    void                               OnCreated() override;
    void                               BindTarget() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty