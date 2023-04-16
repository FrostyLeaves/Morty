/**
 * @File         MEditorModule
 * 
 * @Created      2021-08-20 11:38:47
 *
 * @Author       DoubleYe
**/

#ifndef _M_MEDITORMODULE_H_
#define _M_MEDITORMODULE_H_
#include "Utility/MGlobal.h"

class MEngine;
class MORTY_API MEditorModule
{
public:

	static bool Register(MEngine* pEngine);

};


#endif
