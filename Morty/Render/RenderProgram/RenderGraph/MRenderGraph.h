/**
 * @File         MRenderGraph
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Basic/MTexture.h"
#include "Render/MRenderPass.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MGlobal.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/MeshRender/MIndirectIndexRenderable.h"
#include "TaskGraph/MTaskGraph.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"
#include "Utility/MStringId.h"

class IPropertyBlockAdapter;
class MRenderTaskTarget;
class MRenderTargetManager;

class MORTY_API MRenderGraph : public MTaskGraph
{
    MORTY_CLASS(MRenderGraph)
public:

    MRenderGraph() = default;
    MRenderGraph(MEngine* pEngine);
    ~MRenderGraph() override;

    template<typename TYPE>
    MRenderTaskNode* RegisterTaskNode();

    template<typename TYPE>
    TYPE* FindTaskNode() const;

    MRenderTargetManager* GetRenderTargetManager() const { return m_pRenderTargetManager; }
    MRenderTaskTarget* FindRenderTaskTarget(const MStringId& name);

    void SetFrameProperty(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter) { m_pFramePropertyAdapter = pAdapter; }
    const std::shared_ptr<IPropertyBlockAdapter>& GetFrameProperty() const { return m_pFramePropertyAdapter; }

    void SetCameraCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_pCameraCullingResult = pAdapter; }
    const std::shared_ptr<MInstanceCulling>& GetCameraCullingResult() const { return m_pCameraCullingResult; }

    void SetShadowCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_pShadowCullingResult = pAdapter; }
    const std::shared_ptr<MInstanceCulling>& GetShadowCullingResult() const { return m_pShadowCullingResult; }

    void SetVoxelizerCullingResult(const std::shared_ptr<MInstanceCulling>& pAdapter) { m_pVoxelizerCullingResult = pAdapter; }
    const std::shared_ptr<MInstanceCulling>& GetVoxelizerCullingResult() const { return m_pVoxelizerCullingResult; }

    void Resize(const Vector2i& size);

private:

    Vector2i m_n2Size = {0, 0};

    MEngine* m_pEngine = nullptr;
    MRenderTargetManager* m_pRenderTargetManager = nullptr;

    std::shared_ptr<IPropertyBlockAdapter> m_pFramePropertyAdapter = nullptr;
    std::shared_ptr<MInstanceCulling> m_pCameraCullingResult = nullptr;
    std::shared_ptr<MInstanceCulling> m_pShadowCullingResult = nullptr;
    std::shared_ptr<MInstanceCulling> m_pVoxelizerCullingResult = nullptr;


    std::map<const MType*, MRenderTaskNode*> m_tTaskNodeTable;
};

template <typename TYPE>
MRenderTaskNode* MRenderGraph::RegisterTaskNode()
{
    const auto findResult = m_tTaskNodeTable.find(TYPE::GetClassType());
    if (findResult != m_tTaskNodeTable.end())
    {
        return findResult->second;
    }

    auto typeName = MStringId(TYPE::GetClassTypeName());
    auto pRenderWork = AddNode<TYPE>(typeName);

    m_tTaskNodeTable[TYPE::GetClassType()] = pRenderWork;

    return pRenderWork;
}

template <typename TYPE>
TYPE* MRenderGraph::FindTaskNode() const
{
    const auto findResult = m_tTaskNodeTable.find(TYPE::GetClassType());
    if (findResult != m_tTaskNodeTable.end())
    {
        return static_cast<TYPE*>(findResult->second);
    }

    return nullptr;
}
