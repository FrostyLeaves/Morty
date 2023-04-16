/**
 * @File         MThreadWork
 * 
 * @Created      2021-07-09 10:52:57
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTHREADWORK_H_
#define _M_MTHREADWORK_H_
#include "Utility/MGlobal.h"

enum class METhreadType
{
	EAny = -3,
	EMainThread = -2,
	ECurrentThread = -1,
	ERenderThread = 0,



	ENameThreadNum
};

struct MORTY_API MThreadWork
{
public:
	MThreadWork();

	std::function<void(void)> funcWorkFunction;

	METhreadType eThreadType;
};


#endif
