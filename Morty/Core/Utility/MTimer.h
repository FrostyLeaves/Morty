/**
 * @File         MTimer
 * 
 * @Created      2019-05-19 23:44:18
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTIMER_H_
#define _M_MTIMER_H_
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


#endif
