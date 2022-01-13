/**
 * @File         MRenderModule
 * 
 * @Created      2021-07-19 11:50:59
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERMODULE_H_
#define _M_MRENDERMODULE_H_
#include "MGlobal.h"

class MEngine;
class MORTY_API MRenderModule
{
public:

	static bool Register(MEngine* pEngine);



	static const MString DefaultWhite;
	static const MString DefaultNormal;
	static const MString Default_R8_One;
	static const MString Default_R8_Zero;
};


#endif
