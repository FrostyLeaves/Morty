#pragma once
#include "Utility/MGlobal.h"
#include "SingletonInstance.h"

namespace morty
{

class RenderMessageManager : public SingletonInstance<RenderMessageManager>
{
public:
    size_t nDrawCallCount = 0;
};

}// namespace morty