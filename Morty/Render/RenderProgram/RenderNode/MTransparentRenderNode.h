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
#include "RenderProgram/MRenderInfo.h"

#include <array>

#include "MSinglePassRenderNode.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"

namespace morty
{


class MCullingResultRenderable;
class MTexture;
class MTextureResource;
class MORTY_API MTransparentRenderNode : public ISinglePassRenderNode
{
public:
    MORTY_CLASS(MTransparentRenderNode);

    static const MStringId BackBufferOutput;

public:
    void Initialize(MEngine* pEngine) override;

    void Release() override;

    void Render(const MRenderInfo& info) override;

    void Render(const MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable);

protected:
    void                               InitializeMaterial();

    void                               ReleaseMaterial();

    void                               InitializeFillRenderPass();

    void                               BindTarget() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;


private:
    std::shared_ptr<MMaterial> m_drawFillMaterial = nullptr;
};

}// namespace morty