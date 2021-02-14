/**
 * @File         MIPostProcessWork
 * 
 * @Created      2020-11-30 16:08:08
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIPOSTPROCESSWORK_H_
#define _M_MIPOSTPROCESSWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "Vector.h"

class MITexture;
class MViewport;
class MIRenderer;
class MIRenderProgram;
class MIRenderTexture;
class MTextureRenderTarget;

class MORTY_API MIPostProcessWork : public MObject
{
public:
	M_OBJECT(MIPostProcessWork);
    MIPostProcessWork();
    virtual ~MIPostProcessWork();

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) {};
	virtual void Release() {};


	virtual void OnDelete();

private:
};

#endif
