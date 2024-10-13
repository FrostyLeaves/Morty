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
#include "TaskGraph/MTaskNodeOutput.h"

#include "MRenderTaskNode.h"
#include "RenderProgram/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTaskTarget;
class MRenderPass;
class MORTY_API MRenderTargetManager : public MObject
{
public:
    MORTY_CLASS(MRenderTargetManager);


    MRenderTaskTarget*                     CreateRenderTarget(const MStringId& name);

    MRenderTaskTarget*                     FindRenderTarget(const MStringId& name) const;

    std::shared_ptr<MTexture>              FindRenderTexture(const MStringId& name) const;

    void                                   ResizeRenderTarget(const Vector2i& size);

    std::vector<std::shared_ptr<MTexture>> GetOutputTextures() const;

private:
    std::unordered_map<MStringId, std::unique_ptr<MRenderTaskTarget>> m_renderTaskTable;
};

}// namespace morty