/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Mesh/MMesh.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"

#include <array>

#include "Render/MFrameShaderPropertyBlock.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{


class MCullingResultRenderable;
class MTexture;
class MTextureResource;
class MORTY_API MTransparentRenderNode : public MRenderTaskNode
{
public:
    MORTY_CLASS(MTransparentRenderNode);

public:
    void OnCreated() override;

    void Release() override;

    void Render(const MRenderInfo& info) override;

protected:
    void                               InitializeMaterial();
    void                               ReleaseMaterial();

    void                               InitializeTexture();
    void                               ReleaseTexture();

    void                               InitializeRenderPass();

    void                               DrawPeel(const MRenderInfo& info);
    void                               DrawFill(const MRenderInfo& info);

    void                               BindInOutTexture() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

private:
    std::shared_ptr<MResource>                           m_whiteTexture = nullptr;
    std::shared_ptr<MResource>                           m_blackTexture = nullptr;

    std::shared_ptr<MMaterial>                           m_copyDepthMaterial = nullptr;
    std::shared_ptr<MMaterial>                           m_blendMaterial     = nullptr;

    std::array<std::shared_ptr<MShaderPropertyBlock>, 2> m_framePropertyBlock;

    MRenderPass                                          m_peelPass;
    MRenderPass                                          m_fillPass;
};

}// namespace morty