#include "MTimer.h"
#include <chrono>

MTimer::MTimer()
{

}

MTimer::~MTimer()
{

}

long long MTimer::GetCurTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
