/**
 * @File         MRenderOutputBindingWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "TaskGraph/MTaskGraphWalker.h"

namespace morty
{

class IPropertyBlockAdapter;
class MIMesh;
class MIRenderCommand;
class MTaskNode;
class MTaskGraph;
class MRenderTaskNode;
class MORTY_API MRenderOutputBindingWalker : public ITaskGraphWalker
{
public:
    MRenderOutputBindingWalker() = default;

    void operator()(MTaskGraph* pTaskGraph) override;

private:
    std::unordered_map<MStringId, MRenderTaskNodeOutput*> m_outputs;
};

}// namespace morty