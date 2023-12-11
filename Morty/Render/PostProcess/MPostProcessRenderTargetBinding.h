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

class IPropertyBlockAdapter;
class MShaderPropertyBlock;
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

    std::shared_ptr<IPropertyBlockAdapter> GetFrameProperty() const;

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

    std::shared_ptr<MShaderPropertyBlock> m_pFrameProperty = nullptr;

    MEngine* m_pEngine = nullptr;

    Vector2i m_n2ScreenSize = {1, 1};
};
