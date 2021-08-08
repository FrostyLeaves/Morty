/**
 * @File         MIRenderProgram
 *
 * @Created      2020-07-02 11:43:46
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIRENDERPROGRAM_H_
#define _M_MIRENDERPROGRAM_H_
#include "MGlobal.h"
#include "MObject.h"

class MScene;
class MTexture;
class MViewport;
class MIRenderer;
class MRenderInfo;
class MRenderGraph;
class MIMeshInstance;
class MRenderCommand;
struct MShaderConstantParam;
class MORTY_API MIRenderProgram : public MObject
{
public:
	MORTY_INTERFACE(MIRenderProgram);
    MIRenderProgram();
    virtual ~MIRenderProgram() {}


    void SetViewport(MViewport* pViewport) { m_pViewport = pViewport; }
    MViewport* GetViewport() { return m_pViewport; }

    virtual MTexture* GetOutputTexture() = 0;

private:


    MViewport* m_pViewport;
};

#endif
