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
#include "TaskGraph/MTaskNodeOutput.h"

#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderTargetManager.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskTarget;

struct MRenderTaskOutputDesc {
    MTextureDesc           texture;
    MPassTargetDescription renderDesc;

    MEAllocPolicy          allocPolicy  = MEAllocPolicy::Allocate;
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
    void                                SetRenderTarget(MRenderTaskTarget* pRenderTarget);

    [[nodiscard]] MRenderTaskOutputDesc GetOutputDesc() const;
    [[nodiscard]] MRenderTaskTarget*    GetRenderTarget() const { return m_renderTaskTarget; }

    [[nodiscard]] MTexturePtr           GetTexture() const;

    [[nodiscard]] METextureFormat       GetFormat() const;

    static MRenderTaskOutputDesc        Create(const METextureFormat& format, const MPassTargetDescription& rtDesc);
    static MRenderTaskOutputDesc
    Create(const MTextureDesc& texDesc, const MPassTargetDescription& rtDesc, float scale, size_t texelSize);
    static MRenderTaskOutputDesc
    CreateFixed(const METextureFormat& format, const MPassTargetDescription& rtDesc, const Vector2i& size);
    static MRenderTaskOutputDesc CreateFromInput(const MPassTargetDescription& rtDesc, size_t nInputIdx);

private:
    MRenderTaskTarget* m_renderTaskTarget = nullptr;
};

}// namespace morty