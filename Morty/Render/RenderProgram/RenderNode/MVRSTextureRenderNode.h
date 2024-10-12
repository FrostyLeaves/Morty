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
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderCommon.h"

namespace morty
{

class MORTY_API MVRSTextureRenderNode : public MRenderTaskNode
{
    MORTY_CLASS(MVRSTextureRenderNode)
    static const MStringId VRS_TEXTURE;

public:
    void     Initialize(MEngine* pEngine) override;

    void     Release() override;

    void     Resize(Vector2i size) override;

    void     Render(const MRenderInfo& info) override;

    MEngine* GetEngine() const { return m_engine; }


protected:
    void                               BindTarget() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;

    MEngine*                           m_engine = nullptr;

    MComputeDispatcher*                m_vRSGenerator = nullptr;

    Vector2i                           m_texelSize = {8, 8};
};

}// namespace morty