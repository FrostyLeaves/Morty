/**
 * @File         MRenderTaskNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;

struct MRenderTaskInputDesc {
    METextureFormat       format     = METextureFormat::UNorm_RGBA8;
    bool                  allowEmpty = false;
    METextureBarrierStage barrier    = METextureBarrierStage::EPixelShaderSample;
};

class MORTY_API MRenderTaskNodeInput : public MTaskNodeInput
{
    MORTY_CLASS(MRenderTaskNodeInput)

    void                               SetInputDesc(const MRenderTaskInputDesc& desc) { m_desc = desc; }
    [[nodiscard]] MRenderTaskInputDesc GetInputDesc() const { return m_desc; }
    [[nodiscard]] METextureFormat      GetFormat() const { return m_desc.format; }

    static MRenderTaskInputDesc        CreateSample(METextureFormat format, bool allowEmpty);
    static MRenderTaskInputDesc        CreatePixelWrite(METextureFormat format, bool allowEmpty);
    static MRenderTaskInputDesc        CreateDepth();

private:
    MRenderTaskInputDesc m_desc;
};

}// namespace morty