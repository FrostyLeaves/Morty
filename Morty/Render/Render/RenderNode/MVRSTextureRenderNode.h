/**
 * @File         MVRSTextureRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"

namespace morty
{

class MORTY_API MVRSTextureRenderNode : public MRenderTaskNode
{
    MORTY_CLASS(MVRSTextureRenderNode)

public:
    void OnCreated() override;

    void Release() override;

    void Resize(Vector2i size) override;

    void Render(const MRenderInfo& info) override;


protected:
    void                               BindInOutTexture() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

    MComputeDispatcher*                m_vRSGenerator = nullptr;

    Vector2i                           m_texelSize = {8, 8};

    static const MStringId             VRS_TEXTURE;
};

}// namespace morty