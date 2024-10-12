/**
 * @File         MRenderTaskNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "MRenderTaskNodeOutput.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTaskNodeInput;
class IShaderPropertyUpdateDecorator;
class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskNodeOutput;
class MORTY_API MRenderTaskTarget
{
public:
    enum class ResizePolicy
    {
        Fixed = 0,
        Scale = 1,
    };

    enum class SharedPolicy
    {
        Shared    = 0,
        Exclusive = 1,
    };


    MRenderTaskTarget*               InitResizePolicy(ResizePolicy ePolicy, float fScale = 1.0f, size_t nTexelSize = 1);

    MRenderTaskTarget*               InitSharedPolicy(SharedPolicy ePolicy);

    MRenderTaskTarget*               InitTextureDesc(const MTextureDesc& desc);

    void                             SetTexture(const std::shared_ptr<MTexture>& pTexture);

    const std::shared_ptr<MTexture>& GetTexture() const { return m_texture; }

    SharedPolicy                     GetSharedPolicy() const { return m_sharedPolicy; }

    ResizePolicy                     GetResizePolicy() const { return m_resizePolicy; }

    MTextureDesc                     GetTextureDesc() const { return m_textureDesc; }

    float                            GetScale() const { return m_scale; }

    size_t                           GetTexelSize() const { return m_texelSize; }

private:
    std::shared_ptr<MTexture> m_texture      = nullptr;
    MTextureDesc              m_textureDesc  = {};
    SharedPolicy              m_sharedPolicy = SharedPolicy::Shared;
    ResizePolicy              m_resizePolicy = ResizePolicy::Scale;
    float                     m_scale        = 1.0f;
    size_t                    m_texelSize    = 1;
};

struct MRenderTaskInputDesc {
    MStringId             name;
    METextureBarrierStage barrier = METextureBarrierStage::EPixelShaderSample;
};

struct MRenderTaskOutputDesc {
    MStringId              name;
    MPassTargetDescription renderDesc;
};

class MORTY_API MRenderTaskNode : public MTaskNode
{
    MORTY_CLASS(MRenderTaskNode)
public:
    virtual void                                            Initialize(MEngine* pEngine) { MORTY_UNUSED(pEngine); }

    virtual void                                            Release() {}

    virtual void                                            Render(const MRenderInfo& info) { MORTY_UNUSED(info); }

    virtual void                                            RenderSetup(const MRenderInfo& info) { MORTY_UNUSED(info); }

    virtual void                                            BindTarget() {}

    virtual void                                            RegisterSetting() {}

    virtual void                                            Resize(Vector2i size) { MORTY_UNUSED(size); }

    virtual std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() { return nullptr; }

    virtual std::vector<MRenderTaskInputDesc>               InitInputDesc() { return {}; }

    virtual std::vector<MRenderTaskOutputDesc>              InitOutputDesc() { return {}; }

    void                                                    OnCreated() override;

    void                                                    OnDelete() override;


    MRenderGraph*                                           GetRenderGraph() const;

    MRenderTargetManager*                                   GetRenderTargetManager() const;

    std::shared_ptr<MTexture>                               GetInputTexture(const size_t& nIdx);

    std::shared_ptr<MTexture>                               GetOutputTexture(const size_t& nIdx);

    std::shared_ptr<MTexture>                               GetInputTexture(const MStringId& nIdx);

    std::shared_ptr<MTexture>                               GetOutputTexture(const MStringId& nIdx);


    MRenderTaskNodeOutput*                                  GetRenderOutput(const size_t& nIdx);

    std::unordered_map<MStringId, MRenderTaskNodeInput*>    m_input;
    std::unordered_map<MStringId, MRenderTaskNodeOutput*>   m_output;
};

}// namespace morty