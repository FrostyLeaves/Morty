/**
 * @File         MRenderTargetManager
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "Object/MObject.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MStringId.h"
#include "Flatbuffer/MRenderTaskNode_generated.h"

namespace morty
{

class MRenderTaskTarget;
class MRenderPass;

using MEResizePolicy = morty::fbs::ResizePolicy;
using MESharedPolicy = morty::fbs::SharedPolicy;
using MEAllocPolicy  = morty::fbs::AllocPolicy;

class MORTY_API MRenderTargetManager : public MObject
{
public:
    MORTY_CLASS(MRenderTargetManager);
    using RenderTargetTable = std::unordered_map<MStringId, std::unique_ptr<MRenderTaskTarget>>;

    struct RenderTargetDesc {
        MStringId      name;
        float          scale        = 1.0f;
        MEResizePolicy resizePolicy = MEResizePolicy::Scale;
        MESharedPolicy sharedPolicy = MESharedPolicy::Shared;
        MTextureDesc   textureDesc;
    };

    MRenderTaskTarget*                     CreateRenderTarget(const RenderTargetDesc& desc);
    MRenderTaskTarget*                     CreateRenderTarget(const morty::fbs::MRenderGraphTargetDesc& fbDesc);
    void                                   ResizeRenderTarget(const Vector2i& size);

    [[nodiscard]] MRenderTaskTarget*       FindRenderTarget(const MStringId& name) const;
    [[nodiscard]] MTexturePtr              FindRenderTexture(const MStringId& name) const;
    [[nodiscard]] MTextureArray            GetOutputTextures() const;

    [[nodiscard]] const RenderTargetTable& GetRenderTargetTable() const { return m_renderTaskTable; }


    static flatbuffers::Offset<void>
    SerializeRenderTarget(MRenderTaskTarget* target, flatbuffers::FlatBufferBuilder& builder);

protected:
    MRenderTaskTarget* CreateRenderTarget(const MStringId& name);

private:
    RenderTargetTable m_renderTaskTable;
};

}// namespace morty