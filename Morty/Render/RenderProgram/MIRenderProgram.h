/**
 * @File         MIRenderProgram
 *
 * @Created      2020-07-02 11:43:46
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

MORTY_SPACE_BEGIN

class MTaskGraph;
class MScene;
class MTexture;
class MViewport;
class MIRenderer;
class MRenderGraph;
class MIMeshInstance;
class MIRenderCommand;
struct MRenderInfo;
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

    virtual std::shared_ptr<MTexture> GetOutputTexture() = 0;
    virtual std::vector<std::shared_ptr<MTexture>> GetOutputTextures();
    virtual MTaskGraph* GetRenderGraph() = 0;

private:


    MViewport* m_pViewport;

};

MORTY_SPACE_END