/**
 * @File         MRenderModule
 * 
 * @Created      2021-07-19 11:50:59
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERMODULE_H_
#define _M_MRENDERMODULE_H_
#include "Utility/MGlobal.h"

class MEngine;
class MObject;
class MORTY_API MRenderModule
{
public:

	static bool Register(MEngine* pEngine);

	static void OnObjectPostCreate(MObject* pObject);

	static const MString DefaultWhite;
	static const MString DefaultNormal;
	static const MString Default_R8_One;
	static const MString Default_R8_Zero;
};


#endif
