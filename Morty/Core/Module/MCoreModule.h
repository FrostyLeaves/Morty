/**
 * @File         MCoreModule
 * 
 * @Created      2021-08-03 14:54:21
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"

MORTY_SPACE_BEGIN

class MObject;
class MEngine;
class MORTY_API MCoreModule
{
public:

	static bool Register(MEngine* pEngine);
	static void OnObjectPostCreate(MObject* pObject);
};

MORTY_SPACE_END