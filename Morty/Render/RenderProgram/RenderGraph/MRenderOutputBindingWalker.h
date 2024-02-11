/**
 * @File         MRenderOutputBindingWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Basic/MTexture.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "Utility/MGlobal.h"
#include "TaskGraph/MTaskGraphWalker.h"

class IPropertyBlockAdapter;
class MIMesh;
class MIRenderCommand;
class MTaskNode;
class MTaskGraph;
class MRenderTaskNode;

class MORTY_API MRenderOutputBindingWalker: public ITaskGraphWalker
{
public:
    MRenderOutputBindingWalker() = default;

    void operator ()(MTaskGraph* pTaskGraph) override;

private:

    std::unordered_map<MStringId, MRenderTaskNodeOutput*> m_tOutputs;
};
