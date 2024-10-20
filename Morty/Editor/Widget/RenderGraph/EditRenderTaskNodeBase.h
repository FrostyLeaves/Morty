#pragma once
#include "Utility/MGlobal.h"
#include "Property/PropertyBase.h"

namespace morty
{

class MRenderTaskNode;
class EditRenderTaskNodeBase : public PropertyBase
{
public:
    [[nodiscard]] virtual const MType* GetNodeType() const                              = 0;
    virtual void                       EditRenderTaskNode(MRenderTaskNode* pRenderNode) = 0;
};

}// namespace morty