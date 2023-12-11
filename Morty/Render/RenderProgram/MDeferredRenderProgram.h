/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Culling/MBoundingBoxCulling.h"
#include "Utility/MGlobal.h"

#include "Type/MType.h"
#include "Render/MMesh.h"
#include "Utility/MBounds.h"
#include "Resource/MResource.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskGraph.h"
#include "Render/MRenderPass.h"
#include "MRenderInfo.h"
#include "RenderProgram/MIRenderProgram.h"

#include "MFrameShaderPropertyBlock.h"
#include "Culling/MCascadedShadowCulling.h"
#include "Culling/MInstanceCulling.h"
#include <memory>

class MCPUCameraFrustumCulling;
class MGPUCameraFrustumCulling;
class IGBufferAdapter;
class IPropertyBlockAdapter;
class ITextureInputAdapter;
class MViewport;
class MMaterial;
class MIRenderCommand;
class MComputeDispatcher;
class MRenderMeshComponent;

class MORTY_API MDeferredRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MDeferredRenderProgram)

	MDeferredRenderProgram() = default;
    ~MDeferredRenderProgram() override = default;

public:

	void Render(MIRenderCommand* pPrimaryCommand) override;

	void RenderSetup(MIRenderCommand* pPrimaryCommand);
	
	void RenderGBuffer();

	void RenderLightning();

	void RenderVoxelizer();

	void RenderShadow();

	void RenderForward();

	void RenderVoxelizerDebug();

	void RenderTransparent();

	void RenderPostProcess();

	void RenderDebug();

	void RenderVRS();

	std::shared_ptr<MTexture> GetOutputTexture() override;
	std::vector<std::shared_ptr<MTexture>> GetOutputTextures() override;

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

	void UpdateFrameParams(MRenderInfo& info);


	template<typename TYPE>
	IRenderWork* RegisterRenderWork()
	{
		if (m_tRenderWork.find(TYPE::GetClassType()) != m_tRenderWork.end())
		{
			return m_tRenderWork[TYPE::GetClassType()].get();
		}
		auto pRenderWork = std::make_unique<TYPE>();
		pRenderWork->Initialize(GetEngine());
		if (auto pFramePropertyDecorator = pRenderWork->GetFramePropertyDecorator())
		{
			m_pFramePropertyAdapter->RegisterPropertyDecorator(pFramePropertyDecorator);
		}
		m_tRenderWork[TYPE::GetClassType()] = std::move(pRenderWork);
		return m_tRenderWork[TYPE::GetClassType()].get();
	}

	template<typename TYPE>
	TYPE* GetRenderWork()
	{
		const auto& findResult = m_tRenderWork.find(TYPE::GetClassType());
		if (findResult != m_tRenderWork.end())
		{
			IRenderWork* pRenderWork = findResult->second.get();
			return reinterpret_cast<TYPE*>(pRenderWork);
		}

		return nullptr;
	}

protected:

	MRenderInfo m_renderInfo;

	std::vector<std::shared_ptr<MTexture>> m_vRenderTargets;
	std::shared_ptr<MTexture> m_pFinalOutputTexture = nullptr;
	
	std::shared_ptr<MFrameShaderPropertyBlock> m_pFramePropertyAdapter = nullptr;

	MCullingTaskNode<MCascadedShadowCulling>* m_pShadowCulling = nullptr;

#if GPU_CULLING_ENABLE
	MCullingTaskNode<MGPUCameraFrustumCulling>* m_pCameraFrustumCulling = nullptr;
#else
	MCullingTaskNode<MCPUCameraFrustumCulling>* m_pCameraFrustumCulling = nullptr;
#endif

	MCullingTaskNode<MBoundingBoxCulling>* m_pVoxelizerCulling = nullptr;

	std::unordered_map<const MType*, std::unique_ptr<IRenderWork>> m_tRenderWork;

	uint32_t m_nFrameIndex = 0;


	std::unique_ptr<MTaskGraph> m_pCullingTask;
};
