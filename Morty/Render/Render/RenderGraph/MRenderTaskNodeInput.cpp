#include "MRenderTaskNodeInput.h"
#include "MRenderTaskNode.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeInput, MTaskNodeInput)
MRenderTaskInputDesc MRenderTaskNodeInput::GetInputDesc() const
{
    const size_t nIndex = GetIndex();
    return GetTaskNode()->DynamicCast<MRenderTaskNode>()->InitInputDesc().at(nIndex);
}
