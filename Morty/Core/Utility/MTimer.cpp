#include "Utility/MTimer.h"
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

int MTimer::LocalTime(time_t& time, tm& tmsut)
{
#ifdef MORTY_WIN
	return localtime_s(&tmsut, &time);
#else
	return localtime_r(&time, &tmsut);
#endif
}