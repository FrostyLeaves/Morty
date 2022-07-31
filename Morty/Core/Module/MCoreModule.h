/**
 * @File         MCoreModule
 * 
 * @Created      2021-08-03 14:54:21
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOREMODULE_H_
#define _M_MCOREMODULE_H_
#include "Utility/MGlobal.h"

class MEngine;
class MORTY_API MCoreModule
{
public:

	static bool Register(MEngine* pEngine);

};

#endif
