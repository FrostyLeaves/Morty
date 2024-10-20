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
#include "Render/RenderGraph/MRenderTargetManager.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskTarget;

struct MRenderTaskInputDesc {
    MStringId             name;
    METextureFormat       format  = METextureFormat::UNorm_RGBA8;
    METextureBarrierStage barrier = METextureBarrierStage::EPixelShaderSample;
};

class MORTY_API MRenderTaskNodeInput : public MTaskNodeInput
{
    MORTY_CLASS(MRenderTaskNodeInput)

    [[nodiscard]] MRenderTaskInputDesc GetInputDesc() const;
};

}// namespace morty