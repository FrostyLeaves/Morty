/**
 * @File         MIRenderProgram
 *
 * @Created      2020-07-02 11:43:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIRENDERPROGRAM_H_
#define _M_MIRENDERPROGRAM_H_
#include "MGlobal.h"
#include "MObject.h"

class MScene;
class MViewport;
class MIRenderer;
class MShaderConstantParam;
class MIRenderTarget;
class MIMeshInstance;
class MORTY_CLASS MIRenderProgram : public MObject
{
public:
	M_I_OBJECT(MIRenderProgram);
    MIRenderProgram() {}
    virtual ~MIRenderProgram() {}

public:

    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene, MIRenderTarget* pRenderTarget) {};

    virtual void DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance, MShaderConstantParam* pMeshMatrixParam, MShaderConstantParam* pAnimationParam) {}
};

#endif
