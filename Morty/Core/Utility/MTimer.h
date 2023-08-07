/**
 * @File         MTimer
 * 
 * @Created      2019-05-19 23:44:18
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"


class MORTY_API MTimer
{
public:
    MTimer();
    virtual ~MTimer();

public:

	static long long GetCurTime();

private:

};
