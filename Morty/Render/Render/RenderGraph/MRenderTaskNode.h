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
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph//MRenderTaskNodeInput.h"
#include "Render/RenderGraph//MRenderTaskNodeOutput.h"
#include "Render/RenderGraph/MRenderTargetManager.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MStringId.h"
#include "Flatbuffer/MRenderTaskNode_generated.h"


#define REFL_RENDER_NODE_CLASS    class MORTY_API [[clang::annotate("RenderGraphNode")]]
#define REFL_RENDER_NODE_PROPERTY [[clang::annotate("RenderNodeProperty")]]


namespace morty
{

class MRenderTaskNodeInput;
class IShaderPropertyUpdateDecorator;
class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskNodeOutput;

using MEResizePolicy = morty::fbs::ResizePolicy;
using MESharedPolicy = morty::fbs::SharedPolicy;

class MORTY_API MRenderTaskTarget
{
public:
    MRenderTaskTarget*      InitName(const MStringId& name);
    MRenderTaskTarget*      InitResizePolicy(MEResizePolicy ePolicy, float fScale = 1.0f, size_t nTexelSize = 1);
    MRenderTaskTarget*      InitSharedPolicy(MESharedPolicy ePolicy);
    MRenderTaskTarget*      InitTextureDesc(const MTextureDesc& desc);

    void                    SetTexture(const MTexturePtr& pTexture);

    [[nodiscard]] MStringId GetName() const { return m_name; }
    [[nodiscard]] const MTexturePtr& GetTexture() const { return m_texture; }
    [[nodiscard]] MESharedPolicy     GetSharedPolicy() const { return m_sharedPolicy; }
    [[nodiscard]] MEResizePolicy     GetResizePolicy() const { return m_resizePolicy; }
    [[nodiscard]] MTextureDesc       GetTextureDesc() const { return m_textureDesc; }
    [[nodiscard]] float              GetScale() const { return m_scale; }
    [[nodiscard]] size_t             GetTexelSize() const { return m_texelSize; }

private:
    MStringId      m_name;
    MTexturePtr    m_texture      = nullptr;
    MTextureDesc   m_textureDesc  = {};
    MESharedPolicy m_sharedPolicy = MESharedPolicy::Shared;
    MEResizePolicy m_resizePolicy = MEResizePolicy::Scale;
    float          m_scale        = 1.0f;
    size_t         m_texelSize    = 1;
};

class MORTY_API MRenderTaskNode : public MTaskNode
{
    MORTY_CLASS(MRenderTaskNode)
public:
    virtual void                                            Release() {}
    virtual void                                            Render(const MRenderInfo& info) { MORTY_UNUSED(info); }
    virtual void                                            RenderSetup(const MRenderInfo& info) { MORTY_UNUSED(info); }
    virtual void                                            BindTarget() {}
    virtual void                                            RegisterSetting() {}
    virtual void                                            Resize(Vector2i size) { MORTY_UNUSED(size); }

    flatbuffers::Offset<void>                               Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
    void                                                    Deserialize(const void* flatbuffer) override;

    virtual std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() { return nullptr; }

    virtual std::vector<MRenderTaskInputDesc>               InitInputDesc() { return {}; }
    virtual std::vector<MRenderTaskOutputDesc>              InitOutputDesc() { return {}; }

    void                                                    OnCreated() override;
    void                                                    OnDelete() override;

    [[nodiscard]] MEngine*                                  GetEngine() const;
    [[nodiscard]] MRenderGraph*                             GetRenderGraph() const;
    [[nodiscard]] MRenderTargetManager*                     GetRenderTargetManager() const;
    MTexturePtr                                             GetInputTexture(const size_t& nIdx);
    MTexturePtr                                             GetOutputTexture(const size_t& nIdx);
    MTexturePtr                                             GetInputTexture(const MStringId& nIdx);
    MTexturePtr                                             GetOutputTexture(const MStringId& nIdx);
    MRenderTaskNodeOutput*                                  GetRenderOutput(const size_t& nIdx);

    static METextureFormat                                  DefaultLinearSpaceFormat;

private:
    std::unordered_map<MStringId, MRenderTaskNodeInput*>  m_input;
    std::unordered_map<MStringId, MRenderTaskNodeOutput*> m_output;
};

}// namespace morty