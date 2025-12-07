/**
 * @File         MRenderGraphProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
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

namespace morty
{

class MRenderTargetManager;
class MCPUCameraFrustumCulling;
class MGPUCameraFrustumCulling;
class IGBufferAdapter;
class IParameterSetAdapter;
class ITextureInputAdapter;
class MViewport;
class MMaterial;
class MRenderGraph;
class IRenderCommand;
class MComputeDispatcher;
class MRenderMeshComponent;

class MORTY_API MRenderGraphProgram : public MIRenderProgram
{
public:
    MORTY_CLASS(MRenderGraphProgram)

#if GPU_CULLING_ENABLE
    using CameraFrustumCullingType = MGPUCameraFrustumCulling;
#else
    using CameraFrustumCullingType = MCPUCameraFrustumCulling;
#endif

public:
    void          Update() override;
    void          Render(IRenderCommand* primaryCommand) override;

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
    MRenderInfo                   m_renderInfo;

    uint32_t                      m_frameIndex = 0;

    std::unique_ptr<MRenderGraph> m_renderGraph = nullptr;
};

}// namespace morty