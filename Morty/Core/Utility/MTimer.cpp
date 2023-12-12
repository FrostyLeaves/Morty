#include "Utility/MTimer.h"

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

int MTimer::LocalTime(time_t& time, struct tm& tmsut)
{
#if defined(MORTY_WIN)
	return localtime_s(&tmsut, &time);
#elif defined(MORTY_LINUX)
	tmsut = std::localtime(&time);
	return 0;
#else
	return localtime_r(&time, &tmsut);
#endif
}
