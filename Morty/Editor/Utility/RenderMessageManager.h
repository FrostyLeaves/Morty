#pragma  once
#include "Utility/MGlobal.h"
#include "SingletonInstance.h"

MORTY_SPACE_BEGIN

class RenderMessageManager : public SingletonInstance<RenderMessageManager>
{
public:

    size_t nDrawCallCount = 0;

};

MORTY_SPACE_END