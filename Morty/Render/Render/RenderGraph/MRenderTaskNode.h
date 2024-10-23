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

class MORTY_API MRenderTaskNode : public MTaskNode
{
    MORTY_CLASS(MRenderTaskNode)
public:
    virtual void                                            Release() {}
    virtual void                                            Render(const MRenderInfo& info) { MORTY_UNUSED(info); }
    virtual void                                            RenderSetup(const MRenderInfo& info) { MORTY_UNUSED(info); }
    virtual void                                            BindInOutTexture() {}
    virtual void                                            RegisterSetting() {}
    virtual void                                            Resize(Vector2i size);

    flatbuffers::Offset<void>                               Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
    void                                                    Deserialize(const void* flatbuffer) override;
    void                                                    OnPreCompile() override;

    virtual std::shared_ptr<IShaderPropertyUpdateDecorator> GetFramePropertyDecorator() { return nullptr; }

    virtual std::vector<MRenderTaskInputDesc>               InitInputDesc() { return {}; }
    virtual std::vector<MRenderTaskOutputDesc>              InitOutputDesc() { return {}; }

    void                                                    OnCreated() override;
    void                                                    OnDelete() override;

    [[nodiscard]] MEngine*                                  GetEngine() const;
    [[nodiscard]] MRenderGraph*                             GetRenderGraph() const;
    [[nodiscard]] MTexturePtr                               GetInputTexture(const size_t& nIdx) const;
    [[nodiscard]] MTexturePtr                               GetOutputTexture(const size_t& nIdx) const;
    [[nodiscard]] MRenderTaskNodeOutput*                    GetRenderOutput(const size_t& nIdx) const;
    [[nodiscard]] bool                                      IsValidRenderNode();

    static METextureFormat                                  DefaultLinearSpaceFormat;

private:
    enum ValidCacheFlag
    {
        EUnknow  = 0,
        EInvalid = 1,
        EValid   = 2,
    };
    ValidCacheFlag m_validCacheFlag = ValidCacheFlag::EUnknow;
};

}// namespace morty