/**
 * @File         MSceneCullingNode
 *
 * @Created      2026-01-07 21:18:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Render/MRenderInfo.h"


namespace morty
{

class MRenderPassCmd;
class IRenderer : public MTypeClass
{
public:
    MORTY_INTERFACE(IRenderer)
    virtual ~IRenderer()                                       = default;
    virtual void Execute(MRenderPassCmd* primaryCommand) const = 0;
};


class MIndexedIndirectRenderer : public IRenderer
{

public:
    void           Execute(MRenderPassCmd* primaryCommand) const override;

    const MBuffer* indirectBuffer = nullptr;
    const MBuffer* vertexBuffer   = nullptr;
    const MBuffer* indexBuffer    = nullptr;
    size_t         indirectOffset = 0;
    size_t         instanceCount  = 0;
};

class MIndexedIndirectCountRenderer : public IRenderer
{
public:
    struct DrawCall {
        class MMaterialPass*       pass          = nullptr;
        class MShaderParameterSet* parameterSet  = nullptr;
        size_t                     commandOffset = 0;
        size_t                     countOffset   = 0;
        size_t                     maxCount      = 0;
    };

public:
    void                  Execute(MRenderPassCmd* primaryCommand) const override;

    const MBuffer*        indirectBuffer = nullptr;
    const MBuffer*        vertexBuffer   = nullptr;
    const MBuffer*        indexBuffer    = nullptr;
    const MBuffer*        countBuffer    = nullptr;

    std::vector<DrawCall> drawCalls;
};


}// namespace morty