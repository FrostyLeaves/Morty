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
#include "Type/MColor.h"

class MScene;
class MViewport;
class MIRenderer;
struct MShaderConstantParam;
class MIRenderTarget;
class MIMeshInstance;
class MORTY_API MIRenderProgram : public MObject
{
public:
	M_I_OBJECT(MIRenderProgram);
    MIRenderProgram();
    virtual ~MIRenderProgram() {}

public:

    void BindRenderTarget(MIRenderTarget* pRenderTarget);
    MIRenderTarget* GetRenderTarget() { return m_pRenderTarget; }

    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport) {};

    virtual void SetClearColor(const MColor& cClearColor) {}
    virtual MColor GetClearColor() const { return MColor::Black_T; }

    virtual void Initialize() {}
    virtual void Release() {}

private:

    MIRenderTarget* m_pRenderTarget;
};

#endif
