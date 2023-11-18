/**
 * @File         MPostProcessGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "TaskGraph/MTaskGraphWalker.h"

class MIMesh;
class MIRenderCommand;
class MTaskNode;
class MTaskGraph;
class MPostProcessNode;
class MORTY_API MPostProcessGraphWalker: public ITaskGraphWalker
{
public:

    explicit MPostProcessGraphWalker(MIRenderCommand* pRenderCommand, MIMesh* pScreenMesh);

    void operator ()(MTaskGraph* pTaskGraph) override;


private:

    void RecordCommand(MPostProcessNode* pNode);

    MIRenderCommand* m_pRenderCommand = nullptr;
    MIMesh* m_pScreenMesh = nullptr;
};
