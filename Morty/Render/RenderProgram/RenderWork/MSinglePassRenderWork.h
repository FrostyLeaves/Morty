/**
 * @File         MSinglePassRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Basic/MCameraFrustum.h"
#include "MRenderWork.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"

namespace morty
{

class MORTY_API ISinglePassRenderWork : public MRenderTaskNode
{
    MORTY_INTERFACE(ISinglePassRenderWork)
public:
    void                                   Initialize(MEngine* pEngine) override;

    void                                   Release() override;

    void                                   Resize(Vector2i size) override;

    MEngine*                               GetEngine() const { return m_engine; }

    void                                   SetRenderTarget(const MRenderTargetGroup& renderTarget);

    std::vector<std::shared_ptr<MTexture>> GetBackTextures() const;

    std::shared_ptr<MTexture>              GetDepthTexture() const;

    std::shared_ptr<IGetTextureAdapter>    CreateOutput() const;

protected:
    MRenderTargetGroup                                                AutoBindTarget();

    MRenderTargetGroup                                                AutoBindTargetWithVRS();

    void                                                              AutoBindBarrierTexture();

    void                                                              AutoSetTextureBarrier(MIRenderCommand* pCommand);

    MEngine*                                                          m_engine = nullptr;
    MRenderPass                                                       m_renderPass;
    std::unordered_map<METextureBarrierStage, std::vector<MTexture*>> m_barrierTexture;
};

}// namespace morty