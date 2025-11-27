/**
 * @File         MIRenderProgram
 *
 * @Created      2020-07-02 11:43:46
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Object/MObject.h"

namespace morty
{

class MTaskGraph;
class MScene;
class MTexture;
class MViewport;
class MRenderGraph;
class MIMeshInstance;
class IRenderCommand;
struct MRenderInfo;
struct MShaderUniformParam;

class MORTY_API MIRenderProgram : public MObject
{
public:
                             MORTY_INTERFACE(MIRenderProgram);

    void                     SetViewport(MViewport* pViewport) { m_viewport = pViewport; }

    [[nodiscard]] MViewport* GetViewport() const { return m_viewport; }

    virtual void             Render(IRenderCommand* pPrimaryCommand) = 0;

    virtual MRenderGraph*    GetRenderGraph() = 0;

    virtual void             LoadGraph(const std::vector<MByte>& buffer) = 0;
    virtual void             SaveGraph(std::vector<MByte>& output)       = 0;

private:
    MViewport* m_viewport = nullptr;
};

}// namespace morty