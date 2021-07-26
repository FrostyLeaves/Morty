/**
 * @File         MThreadWork
 * 
 * @Created      2021-07-09 10:52:57
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTHREADWORK_H_
#define _M_MTHREADWORK_H_
#include "MGlobal.h"

enum class METhreadType
{
	EAny = -2,
	EMainThread = -1,
	ERenderThread = 0
};

struct MORTY_API MThreadWork
{
public:
	MThreadWork();

	std::function<void(void)> funcWorkFunction;

	METhreadType eThreadType;
};


#endif
