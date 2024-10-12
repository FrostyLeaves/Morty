/**
 * @File         MTaskGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

namespace morty
{

class MTaskNode;
class MTaskGraph;
class MORTY_API ITaskGraphWalker
{
public:
    virtual ~ITaskGraphWalker()                     = default;
    virtual void operator()(MTaskGraph* pTaskGraph) = 0;
};

}// namespace morty