/**
 * @File         MThreadWork
 * 
 * @Created      2021-07-09 10:52:57
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

enum class METhreadType
{
	EAny = -2,
	ECurrentThread = -1,

	EMainThread = 0,
	ERenderThread = 1,

	ENameThreadNum
};

struct MORTY_API MThreadWork
{
public:
	MThreadWork();

	std::function<void(void)> funcWorkFunction;

	METhreadType eThreadType;
};
