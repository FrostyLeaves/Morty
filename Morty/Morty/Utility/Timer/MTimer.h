/**
 * @File         MTimer
 * 
 * @Created      2019-05-19 23:44:18
 *
 * @Author       Morty
**/

#ifndef _M_MTIMER_H_
#define _M_MTIMER_H_
#include "MGlobal.h"


class MORTY_CLASS MTimer
{
public:
    MTimer();
    virtual ~MTimer();

public:

	static long long GetCurTime();

private:

};


#endif
