/**
 * @File         IRenderCommand
 * 
 * @Created      2021-07-14 18:08:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Math/Vector.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/Command/MRenderPassCmdDefine.h"
#include "Utility/MColor.h"

namespace morty
{

class MIMesh;
class MBuffer;
class MTexture;
class MMaterial;
class MRenderPass;
class MRenderPassCmd;
class MShaderPropertyBlock;
class MComputeDispatcher;
class MMaterialPipelineLayoutData;

class MORTY_API IRenderCommand
{
public:
    virtual ~IRenderCommand() = default;

    virtual void           RenderCommandBegin() = 0;

    virtual void           RenderCommandEnd() = 0;

    virtual MRenderPassCmd BeginRenderPass(MRenderPass* renderPass) = 0;

    virtual void           EndRenderPass(const MRenderPassCmd& command) = 0;

    virtual bool           DispatchComputeJob(
                      MComputeDispatcher* pMaterial,
                      const uint32_t&     nGroupX,
                      const uint32_t&     nGroupY,
                      const uint32_t&     nGroupZ
              ) = 0;

    virtual bool AddBufferMemoryBarrier(
            const std::vector<const MBuffer*>& vBuffers,
            MEBufferBarrierStage               srcStage,
            MEBufferBarrierStage               dstStage
    ) = 0;

    virtual bool DownloadTexture(
            MTexture*                                                         pTexture,
            const uint32_t&                                                   unMipIdx,
            const std::function<void(void* pImageData, const Vector2& size)>& callback
    ) = 0;

    virtual bool         CopyImageBuffer(MTexture* pSource, MTexture* pDest) = 0;

    virtual void         ResetBuffer(const MBuffer* pBuffer) = 0;

    virtual void         UploadBuffer(MBuffer* pBuffer, const MByte* pData, size_t nSize) = 0;

    virtual bool         IsFinished() { return false; }

    virtual void         OnCommandFinished() {}

    virtual void         addFinishedCallback(std::function<void()> func) = 0;

    [[nodiscard]] size_t GetDrawCallCount() const { return m_drawCallCount; }

public:
    uint32_t m_unFrameIndex  = 0;
    size_t   m_drawCallCount = 0;
};

}// namespace morty