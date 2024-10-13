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
#include "TaskGraph/MTaskNodeInput.h"

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
class MORTY_API MRenderTaskNodeInput : public MTaskNodeInput
{
    MORTY_CLASS(MRenderTaskNodeInput)
public:
    void      SetName(const MStringId& name) { m_strOutputName = name; }

    MStringId GetName() const { return m_strOutputName; }

private:
    MStringId m_strOutputName;
};

}// namespace morty