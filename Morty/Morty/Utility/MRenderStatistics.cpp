#include "MRenderStatistics.h"

MRenderStatistics* MRenderStatistics::GetInstance()
{
	static MRenderStatistics instance;
	return &instance;
}
