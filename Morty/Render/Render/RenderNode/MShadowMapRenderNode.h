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

class MORTY_API MShadowMapRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MShadowMapRenderNode)
    static const MStringId ShadowMapBufferOutput;

public:
    void                                Initialize(MEngine* pEngine) override;

    void                                Render(const MRenderInfo& info) override;
    void                                Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable);

    std::shared_ptr<IGetTextureAdapter> GetShadowMap() const;

    std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() override;

protected:
    void                               OnCreated() override;
    void                               BindTarget() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;
};

}// namespace morty