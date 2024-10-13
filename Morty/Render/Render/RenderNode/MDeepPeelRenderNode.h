/**
 * @File         MDeepPeelRenderNode
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

#include "Render/MFrameShaderPropertyBlock.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

namespace morty
{

class MCullingResultRenderable;
class MTexture;
class MTextureResource;
class MORTY_API MDeepPeelRenderNode : public ISinglePassRenderNode
{
public:
    MORTY_CLASS(MDeepPeelRenderNode);

    static const MStringId FrontTextureOutput;
    static const MStringId BackTextureOutput;
    static const MStringId DepthOutput[4];

public:
    void Initialize(MEngine* pEngine) override;

    void Release() override;

    void Render(const MRenderInfo& info) override;

    void Render(const MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable);

protected:
    void                               InitializeMaterial();

    void                               ReleaseMaterial();

    void                               InitializeTexture();

    void                               ReleaseTexture();

    void                               InitializeRenderPass();

    void                               InitializeFrameShaderParams();

    void                               ReleaseFrameShaderParams();

    void                               BindTarget() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;


private:
    std::shared_ptr<MTextureResource>                    m_whiteTexture;
    std::shared_ptr<MTextureResource>                    m_blackTexture;

    std::shared_ptr<MMaterial>                           m_drawPeelMaterial;
    std::shared_ptr<MMaterial>                           m_forwardMaterial;

    std::array<std::shared_ptr<MShaderPropertyBlock>, 2> m_framePropertyBlock;
};

}// namespace morty