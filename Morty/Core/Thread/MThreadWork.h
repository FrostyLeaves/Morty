/**
 * @File         MThreadWork
 * 
 * @Created      2021-07-09 10:52:57
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

enum class METhreadType : int
{
	EAny = -2,
	ECurrentThread = -1,

	EMainThread = 0,
	ERenderThread = 1,

	ENameThreadNum
};

struct MORTY_API MThreadWork
{
	MThreadWork() = default;
	MThreadWork(METhreadType type) : eThreadType(static_cast<int>(type)){}

	std::function<void(void)> funcWorkFunction = nullptr;

	int eThreadType = static_cast<int>(METhreadType::EAny);
};
