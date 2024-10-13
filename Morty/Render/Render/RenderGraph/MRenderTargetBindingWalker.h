/**
 * @File         MRenderTargetBindingWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "TaskGraph/MTaskGraphWalker.h"

namespace morty
{

class IPropertyBlockAdapter;
class MIMesh;
class MIRenderCommand;
class MTaskNode;
class MTaskGraph;
class MRenderTaskNode;
class MORTY_API MRenderTargetBindingWalker : public ITaskGraphWalker
{
public:
    MRenderTargetBindingWalker() = default;

    explicit MRenderTargetBindingWalker(MEngine* pEngine);

    ~MRenderTargetBindingWalker();

    void operator()(MTaskGraph* pTaskGraph) override;

private:
    void                           AllocRenderTarget(MRenderTaskNode* pNode);

    void                           FreeRenderTarget(MRenderTaskNode* pNode);

    void                           AllocRenderTarget(MRenderTaskTarget* pRenderTarget);

    void                           FreeRenderTarget(MRenderTaskTarget* pRenderTarget);

    bool                           IsAllPrevNodeHasAlloced(MRenderTaskNode* pNode);

    bool                           IsAllNextNodeHasAlloced(MRenderTaskNode* pNode);

    bool                           IsNodeHasAlloced(MRenderTaskNode* pNode);

    MEngine*                       m_engine = nullptr;

    class MRenderTargetCacheQueue* m_cacheQueue = nullptr;

    enum class AllocState
    {
        Alloced = 0,
        Free    = 1
    };
    std::map<MTaskNode*, AllocState>       m_allocedNode;
    std::map<MRenderTaskTarget*, size_t>   m_targetAllocCount;

    std::vector<std::shared_ptr<MTexture>> m_exclusiveTextures;
};

}// namespace morty