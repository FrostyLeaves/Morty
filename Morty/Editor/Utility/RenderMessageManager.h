#pragma  once
#include "Utility/MGlobal.h"
#include "SingletonInstance.h"

class RenderMessageManager : public SingletonInstance<RenderMessageManager>
{
public:

    size_t nDrawCallCount = 0;

};