/**
 * @File         MIRenderProgram
 *
 * @Created      2020-07-02 11:43:46
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIRENDERPROGRAM_H_
#define _M_MIRENDERPROGRAM_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

class MScene;
class MTexture;
class MViewport;
class MIRenderer;
class MRenderInfo;
class MRenderGraph;
class MIMeshInstance;
class MIRenderCommand;
struct MShaderConstantParam;
class MORTY_API MIRenderProgram : public MObject
{
public:
	MORTY_INTERFACE(MIRenderProgram);
    MIRenderProgram();
    virtual ~MIRenderProgram() {}

    void SetViewport(MViewport* pViewport) { m_pViewport = pViewport; }
    MViewport* GetViewport() { return m_pViewport; }


	virtual void Render(MIRenderCommand* pPrimaryCommand) = 0;

    virtual MTexture* GetOutputTexture() = 0;
    virtual std::vector<MTexture*> GetOutputTextures();

private:


    MViewport* m_pViewport;
};

#endif
