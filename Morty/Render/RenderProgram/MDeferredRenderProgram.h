/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Culling/MBoundingBoxCulling.h"
#include "RenderProgram/RenderGraph/MRenderTargetBindingWalker.h"
#include "Utility/MGlobal.h"

#include "Type/MType.h"
#include "Render/MMesh.h"
#include "Utility/MBounds.h"
#include "Resource/MResource.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskGraph.h"
#include "RenderGraph/MRenderGraph.h"
#include "Render/MRenderPass.h"
#include "MRenderInfo.h"
#include "RenderProgram/MIRenderProgram.h"

#include "MFrameShaderPropertyBlock.h"
#include "Culling/MCascadedShadowCulling.h"
#include "Culling/MInstanceCulling.h"
#include <memory>

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

	void Render(MIRenderCommand* pPrimaryCommand) override;

	void RenderSetup(MIRenderCommand* pPrimaryCommand);
	
	std::shared_ptr<MTexture> GetOutputTexture() override;
	std::vector<std::shared_ptr<MTexture>> GetOutputTextures() override;
	MTaskGraph* GetRenderGraph() override { return m_pRenderGraph.get(); }

public:
	void OnCreated() override;
	void OnDelete() override;

	void InitializeRenderTarget();
	void ReleaseRenderTarget();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();

	void InitializeRenderWork();
	void ReleaseRenderWork();

	void InitializeTaskGraph();
	void ReleaseTaskGraph();
	
protected:


	template<typename TYPE>
	MRenderTaskNode* RegisterRenderWork()
	{
		MRenderTaskNode* pRenderWork = m_pRenderGraph->FindTaskNode<TYPE>();
		if(pRenderWork != nullptr)
		{
			return pRenderWork;
		}

		pRenderWork = m_pRenderGraph->RegisterTaskNode<TYPE>();
		if (pRenderWork)
		{
			pRenderWork->Initialize(GetEngine());
			if (auto pFramePropertyDecorator = pRenderWork->GetFramePropertyDecorator())
			{
				m_pFramePropertyAdapter->RegisterPropertyDecorator(pFramePropertyDecorator);
			}
		}
		return pRenderWork;
	}

	template<typename TYPE>
	TYPE* GetRenderWork() const
	{
		return m_pRenderGraph->FindTaskNode<TYPE>();
	}
	

protected:

	MRenderInfo m_renderInfo;

	std::shared_ptr<MFrameShaderPropertyBlock> m_pFramePropertyAdapter = nullptr;

	MCullingTaskNode<MCascadedShadowCulling>* m_pShadowCulling = nullptr;

#if GPU_CULLING_ENABLE
	MCullingTaskNode<MGPUCameraFrustumCulling>* m_pCameraFrustumCulling = nullptr;
#else
	MCullingTaskNode<MCPUCameraFrustumCulling>* m_pCameraFrustumCulling = nullptr;
#endif

	MCullingTaskNode<MBoundingBoxCulling>* m_pVoxelizerCulling = nullptr;

	uint32_t m_nFrameIndex = 0;


	std::unique_ptr<MTaskGraph> m_pCullingTask = nullptr;
	std::unique_ptr<MRenderGraph> m_pRenderGraph = nullptr;

	std::unique_ptr<MRenderTargetBindingWalker> m_pRenderTargetBinding = nullptr;
};
