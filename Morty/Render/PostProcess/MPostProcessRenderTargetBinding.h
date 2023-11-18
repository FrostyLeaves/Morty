/**
 * @File         MPostProcessRenderTargetBinding
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "TaskGraph/MTaskGraphWalker.h"

class MRenderPass;
class MPostProcessNode;
class MTexture;
class MTaskNodeOutput;
class MIMesh;
class MIRenderCommand;
class MTaskNode;
class MTaskGraph;

class MORTY_API MPostProcessRenderTargetBinding : public ITaskGraphWalker
{
public:

    explicit MPostProcessRenderTargetBinding(MEngine* pEngine);

    void operator ()(MTaskGraph* pTaskGraph) override;

    void Resize(const Vector2i size);

    void Release();

private:

    void AllocRenderTarget(MPostProcessNode* pNode);
    void FreeRenderTarget(MPostProcessNode* pNode);

    bool IsAllPrevNodeHasAlloced(MPostProcessNode* pNode);
    bool IsAllNextNodeHasAlloced(MPostProcessNode* pNode);

    enum class AllocState{ Alloced = 0, Free = 1 };
    std::map<MTaskNode*, AllocState> m_tAllocedNode;

    std::queue<std::shared_ptr<MTexture>> m_vTextures;
    std::vector<std::shared_ptr<MTexture>> m_vAllTextures;
    std::vector<MRenderPass*> m_vAllRenderPass;

    MEngine* m_pEngine = nullptr;
};
