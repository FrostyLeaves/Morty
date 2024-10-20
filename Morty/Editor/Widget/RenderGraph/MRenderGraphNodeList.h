#pragma once

#include "Utility/MRenderGlobal.h"

namespace morty
{

class EditRenderTaskNodeBase;
class MORTY_API MRenderGraphNodeList
{
public:
    using EditCreateFunc = std::function<morty::EditRenderTaskNodeBase*()>;

    static const std::vector<MStringId>                        Names;

    static const std::unordered_map<MStringId, EditCreateFunc> EditFactory;
};

}// namespace morty
