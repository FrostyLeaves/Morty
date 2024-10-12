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

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;
class MRenderTaskTarget;
class MORTY_API MRenderTaskNodeOutput : public MTaskNodeOutput
{
    MORTY_CLASS(MRenderTaskNodeOutput)
public:
    void                      SetName(const MStringId& name) { m_strOutputName = name; }

    MStringId                 GetName() const { return m_strOutputName; }

    void                      SetRenderTarget(MRenderTaskTarget* pRenderTarget);

    MRenderTaskTarget*        GetRenderTarget() const { return m_renderTaskTarget; }

    std::shared_ptr<MTexture> GetTexture() const;

private:
    MStringId          m_strOutputName;
    MRenderTaskTarget* m_renderTaskTarget = nullptr;
};

}// namespace morty