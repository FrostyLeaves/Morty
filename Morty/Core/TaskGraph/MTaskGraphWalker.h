/**
 * @File         MTaskGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

MORTY_SPACE_BEGIN

class MTaskNode;
class MTaskGraph;
class MORTY_API ITaskGraphWalker
{
public:
    virtual ~ITaskGraphWalker() = default;
    virtual void operator ()(MTaskGraph* pTaskGraph) = 0;
};

MORTY_SPACE_END