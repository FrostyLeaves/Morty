#pragma once

#include "Utility/MRenderGlobal.h"
#include "Property/MComponentProperty.h"

namespace morty
{

class MORTY_API MComponentPropertyList
{
public:
    using PropertyCreateFunc = std::function<morty::MComponentProperty*()>;

    static const std::unordered_map<MStringId, PropertyCreateFunc> EditFactory;
};

}// namespace morty
