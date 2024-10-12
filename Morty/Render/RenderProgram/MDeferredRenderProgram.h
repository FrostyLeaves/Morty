/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Culling/MBoundingBoxCulling.h"
#include "RenderProgram/RenderGraph/MRenderTargetBindingWalker.h"

#include "MRenderInfo.h"
#include "Mesh/MMesh.h"
#include "RHI/MRenderPass.h"
#include "RenderGraph/MRenderGraph.h"
#include "RenderProgram/MIRenderProgram.h"
#include "Resource/MResource.h"
#include "TaskGraph/MTaskGraph.h"
#include "TaskGraph/MTaskNode.h"
#include "Type/MType.h"
#include "Utility/MBounds.h"

#include "Culling/MCascadedShadowCulling.h"
#include "Culling/MInstanceCulling.h"
#include "MFrameShaderPropertyBlock.h"

namespace morty
{

class MRenderTargetManager;
class MCPUCameraFrustumCulling;
class MGPUCameraFrustumCulling;
class IGBufferAdapter;
class IPropertyBlockAdapter;
class ITextureInputAdapter;
class MViewport;
class MMaterial;
class MRenderGraph;
class MIRenderCommand;
class MComputeDispatcher;
class MRenderMeshComponent;
class MORTY_API MDeferredRenderProgram : public MIRenderProgram
{
public:
    MORTY_CLASS(MDeferredRenderProgram)

public:
    void                                   Render(MIRenderCommand* pPrimaryCommand) override;

    void                                   RenderSetup(MIRenderCommand* pPrimaryCommand);

    std::shared_ptr<MTexture>              GetOutputTexture() override;

    std::vector<std::shared_ptr<MTexture>> GetOutputTextures() override;

    MTaskGraph*                            GetRenderGraph() override { return m_renderGraph.get(); }

public:
    void OnCreated() override;

    void OnDelete() override;

    void InitializeRenderGraph();

    void ReleaseRenderGraph();

    void InitializeFrameShaderParams();

    void ReleaseFrameShaderParams();

    void InitializeRenderWork();

    void ReleaseRenderWork();

    void InitializeTaskGraph();

    void ReleaseTaskGraph();

protected:
    template<typename TYPE> TYPE* RegisterRenderWork(const MStringId strTaskNodeName)
    {
        MRenderTaskNode* pRenderWork = m_renderGraph->FindTaskNode<TYPE>(strTaskNodeName);
        if (pRenderWork != nullptr) { return static_cast<TYPE*>(pRenderWork); }

        pRenderWork = m_renderGraph->RegisterTaskNode<TYPE>(strTaskNodeName);
        if (pRenderWork)
        {
            pRenderWork->Initialize(GetEngine());
            if (auto pFramePropertyDecorator = pRenderWork->GetFramePropertyDecorator())
            {
                m_framePropertyAdapter->RegisterPropertyDecorator(pFramePropertyDecorator);
            }
        }
        return static_cast<TYPE*>(pRenderWork);
    }

    template<typename TYPE> TYPE* GetRenderWork(const MStringId& strTaskNodeName) const
    {
        return m_renderGraph->FindTaskNode<TYPE>(strTaskNodeName);
    }

    template<typename TYPE> TYPE* RegisterRenderWork()
    {
        return RegisterRenderWork<TYPE>(MStringId(TYPE::GetClassTypeName()));
    }

    template<typename TYPE> TYPE* GetRenderWork() const
    {
        return GetRenderWork<TYPE>(MStringId(TYPE::GetClassTypeName()));
    }


protected:
    MRenderInfo                                m_renderInfo;

    std::shared_ptr<MFrameShaderPropertyBlock> m_framePropertyAdapter = nullptr;

    MCullingTaskNode<MCascadedShadowCulling>*  m_shadowCulling = nullptr;

#if GPU_CULLING_ENABLE
    MCullingTaskNode<MGPUCameraFrustumCulling>* m_cameraFrustumCulling = nullptr;
#else
    MCullingTaskNode<MCPUCameraFrustumCulling>* m_cameraFrustumCulling = nullptr;
#endif

    MCullingTaskNode<MBoundingBoxCulling>*      m_voxelizerCulling = nullptr;

    uint32_t                                    m_frameIndex = 0;


    std::unique_ptr<MTaskGraph>                 m_cullingTask = nullptr;
    std::unique_ptr<MRenderGraph>               m_renderGraph = nullptr;

    std::unique_ptr<MRenderTargetBindingWalker> m_renderTargetBinding = nullptr;
};

}// namespace morty