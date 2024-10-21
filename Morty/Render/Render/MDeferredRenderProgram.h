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
#include "Render/RenderGraph/MRenderTargetBindingWalker.h"

#include "MRenderInfo.h"
#include "Mesh/MMesh.h"
#include "RHI/MRenderPass.h"
#include "Render/MIRenderProgram.h"
#include "RenderGraph/MRenderGraph.h"
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

#if GPU_CULLING_ENABLE
    using CameraFrustumCullingType = MGPUCameraFrustumCulling;
#else
    using CameraFrustumCullingType = MCPUCameraFrustumCulling;
#endif

public:
    void          Render(MIRenderCommand* pPrimaryCommand) override;
    void          RenderSetup(MIRenderCommand* pPrimaryCommand);

    MTexturePtr   GetOutputTexture() override;
    MTextureArray GetOutputTextures() override;
    MRenderGraph* GetRenderGraph() override { return m_renderGraph.get(); }

public:
    void OnCreated() override;
    void OnDelete() override;
    void LoadGraph(const std::vector<MByte>& buffer) override;
    void SaveGraph(std::vector<MByte>& output) override;

    void InitializeRenderGraph();
    void ReleaseRenderGraph();
    void InitializeFrameShaderParams();
    void ReleaseFrameShaderParams();
    void InitializeTaskGraph();
    void ReleaseTaskGraph();


protected:
    MRenderInfo                                 m_renderInfo;

    std::shared_ptr<MFrameShaderPropertyBlock>  m_framePropertyAdapter = nullptr;

    MCullingTaskNode<MCascadedShadowCulling>*   m_shadowCulling        = nullptr;
    MCullingTaskNode<CameraFrustumCullingType>* m_cameraFrustumCulling = nullptr;
    MCullingTaskNode<MBoundingBoxCulling>*      m_voxelizerCulling     = nullptr;

    uint32_t                                    m_frameIndex = 0;

    std::unique_ptr<MTaskGraph>                 m_cullingTask = nullptr;
    std::unique_ptr<MRenderGraph>               m_renderGraph = nullptr;
};

}// namespace morty