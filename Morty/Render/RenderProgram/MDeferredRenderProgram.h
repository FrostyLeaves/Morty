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

#include "Shadow/MShadowMapShaderPropertyBlock.h"
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
	
protected:

	void UpdateFrameParams(MRenderInfo& info);


	template<typename TYPE>
	IRenderWork* RegisterRenderWork()
	{
		if (m_tRenderWork.find(TYPE::GetClassType()) != m_tRenderWork.end())
		{
			return m_tRenderWork[TYPE::GetClassType()].get();
		}
		m_tRenderWork[TYPE::GetClassType()] = std::make_unique<TYPE>();
		m_tRenderWork[TYPE::GetClassType()]->Initialize(GetEngine());
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
	
	std::shared_ptr<MShadowMapShaderPropertyBlock> m_pShadowPropertyAdapter = nullptr;
	std::shared_ptr<MFrameShaderPropertyBlock> m_pFramePropertyAdapter = nullptr;

	std::shared_ptr<MCascadedShadowCulling> m_pShadowCulling = nullptr;
	std::shared_ptr<MCameraFrustumCulling> m_pCameraFrustumCulling = nullptr;
	std::shared_ptr<MBoundingBoxCulling> m_pVoxelizerCulling = nullptr;

	std::unordered_map<const MType*, std::unique_ptr<IRenderWork>> m_tRenderWork;

	size_t m_nFrameIndex = 0;
};
