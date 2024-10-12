/**
 * @File         MTimer
 * 
 * @Created      2019-05-19 23:44:18
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

namespace morty
{

class MORTY_API MTimer
{
public:
    MTimer();

    virtual ~MTimer();

    static long long GetCurTime();

    static int       LocalTime(time_t& time, struct tm& tmsut);
};

}// namespace morty