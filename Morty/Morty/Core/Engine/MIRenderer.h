/**
 * @File         MIRenderer
 * 
 * @Created      2019-05-12 21:49:13
 *
 * @Author       Morty
**/

#ifndef _M_MIRENDERER_H_
#define _M_MIRENDERER_H_
#include "MGlobal.h"

class MIRenderView;
class MIShader;
class MORTY_CLASS MIRenderer
{
public:
	MIRenderer(){};
	virtual ~MIRenderer(){};

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void Render() = 0;

};


#endif
