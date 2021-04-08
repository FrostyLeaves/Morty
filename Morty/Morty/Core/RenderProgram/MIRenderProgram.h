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
#include "Type/MColor.h"

class MScene;
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
	M_I_OBJECT(MIRenderProgram);
    MIRenderProgram();
    virtual ~MIRenderProgram() {}

public:

	virtual MRenderGraph* GetRenderGraph() { return nullptr; }

    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MRenderCommand* pCommand) {};

    virtual void SetClearColor(const MColor& cClearColor) {}
    virtual MColor GetClearColor() const { return MColor::Black_T; }

    virtual void Initialize() {}
    virtual void Release() {}

	virtual MRenderInfo* GetRenderInfo() { return nullptr; }

private:
};

#endif
