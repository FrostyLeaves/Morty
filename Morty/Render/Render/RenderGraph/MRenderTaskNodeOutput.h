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
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MStringId.h"
#include "Flatbuffer/MRenderTaskNode_generated.h"

namespace morty
{

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskTarget;

using METextureSourceType = morty::fbs::TextureSourceType;
using MEResizePolicy      = morty::fbs::ResizePolicy;
using MESharedPolicy      = morty::fbs::SharedPolicy;

struct MRenderTaskOutputDesc {
    MTextureDesc           texture;
    MPassTargetDescription renderDesc;

    METextureSourceType    allocPolicy  = METextureSourceType::Allocate;
    MESharedPolicy         sharedPolicy = MESharedPolicy::Shared;
    MEResizePolicy         resizePolicy = MEResizePolicy::Scale;
    float                  scale        = 1.0f;
    size_t                 texelSize    = 1;
    size_t                 inputIdx     = 0;
};

class MORTY_API MRenderTaskNodeOutput : public MTaskNodeOutput
{
    MORTY_CLASS(MRenderTaskNodeOutput)
public:
    void                                 SetOutputDesc(const MRenderTaskOutputDesc& desc) { m_desc = desc; }
    [[nodiscard]] MRenderTaskOutputDesc  GetOutputDesc() const { return m_desc; }


    void                                 SetRenderTexture(const MTexturePtr& pTexture) { m_renderTexture = pTexture; }
    [[nodiscard]] MTexturePtr            GetRenderTexture() const { return m_renderTexture; }
    [[nodiscard]] METextureFormat        GetFormat() const;
    [[nodiscard]] MRenderTaskNodeOutput* GetActualOutput();

    bool                                 CanLink(const MTaskNodeInput* pInput) const override;

public:
    static MRenderTaskOutputDesc Create(const METextureFormat& format, const MPassTargetDescription& rtDesc);
    static MRenderTaskOutputDesc
    Create(const MTextureDesc& texDesc, const MPassTargetDescription& rtDesc, float scale, size_t texelSize);
    static MRenderTaskOutputDesc
    CreateFixed(const METextureFormat& format, const MPassTargetDescription& rtDesc, const Vector2i& size);
    static MRenderTaskOutputDesc CreateFromInput(const MPassTargetDescription& rtDesc, size_t nInputIdx);

private:
    MRenderTaskOutputDesc m_desc;
    MTexturePtr           m_renderTexture;
};

}// namespace morty