/**
 * @File         MSinglePassRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Object/MObject.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderTaskNode.h"

namespace morty
{
class IGetTextureAdapter;

class MORTY_API ISinglePassRenderNode : public MRenderTaskNode
{
    MORTY_INTERFACE(ISinglePassRenderNode)

    ISinglePassRenderNode();

public:
    void Release() override;
    void Resize(Vector2i size) override;

    void SetRenderTarget(const MRenderTargetGroup& renderTarget);

protected:
    MRenderTargetGroup AutoBindTarget();
    MRenderTargetGroup AutoBindTargetWithVRS();
    void               BindInOutTexture() override;

    MRenderPass        m_renderPass;
};

}// namespace morty