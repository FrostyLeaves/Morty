/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MDEFERRED_RENDERPROGRAM_H_
#define _M_MDEFERRED_RENDERPROGRAM_H_
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
#include "MForwardRenderShaderPropertyBlock.h"
#include "Culling/MCascadedShadowCulling.h"
#include "Culling/MInstanceCulling.h"
#include "RenderWork/MDebugRenderWork.h"
#include "RenderWork/MDeferredLightingRenderWork.h"
#include "RenderWork/MForwardRenderWork.h"
#include "RenderWork/MGBufferRenderWork.h"
#include "RenderWork/MPostProcessRenderWork.h"
#include "RenderWork/MShadowMapRenderWork.h"
#include "RenderWork/MTransparentRenderWork.h"

class MSceneCulling;
class MSceneGPUCulling;
class IGBufferAdapter;
class IPropertyBlockAdapter;
class ITextureInputAdapter;
class MViewport;
class MMaterial;
class MIRenderCommand;
class MComputeDispatcher;
class MRenderableMeshComponent;
class MORTY_API MDeferredRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MDeferredRenderProgram);

	MDeferredRenderProgram() = default;
    virtual ~MDeferredRenderProgram() = default;

public:

	virtual void Render(MIRenderCommand* pPrimaryCommand) override;

	void RenderReady(MTaskNode* pTaskNode);
	
	void RenderGBuffer(MTaskNode* pTaskNode);

	void RenderLightning(MTaskNode* pTaskNode);

	void RenderShadow(MTaskNode* pTaskNode);

	void RenderForward(MTaskNode* pTaskNode);

	void RenderTransparent(MTaskNode* pTaskNode);

	void RenderPostProcess(MTaskNode* pTaskNode);

	void RenderDebug(MTaskNode* pTaskNode);


	virtual std::shared_ptr<MTexture> GetOutputTexture() override;
	virtual std::vector<std::shared_ptr<MTexture>> GetOutputTextures() override;

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void InitializeRenderTarget();
	void ReleaseRenderTarget();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();

	void InitializeRenderWork();
	void ReleaseRenderWork();

	void InitializeRenderGraph();

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

	MTaskGraph* m_pRenderGraph = nullptr;
	MIRenderCommand* m_pPrimaryCommand = nullptr;



	std::vector<std::shared_ptr<MTexture>> m_vRenderTargets;
	std::shared_ptr<MTexture> m_pFinalOutputTexture = nullptr;

protected:

	ITextureInputAdapter* m_pShadowMapAdapter = nullptr;
	std::unique_ptr<IGBufferAdapter> m_pGBufferAdapter = nullptr;

	std::shared_ptr<MShadowMapShaderPropertyBlock> m_pShadowPropertyAdapter = nullptr;
	std::shared_ptr<MForwardRenderShaderPropertyBlock> m_pFramePropertyAdapter = nullptr;

	std::shared_ptr<CameraFrustumCulling> m_pCameraFrustumCulling = nullptr;
	std::shared_ptr<MCascadedShadowCulling> m_pShadowCulling = nullptr;
	std::shared_ptr<MSceneCulling> m_pCpuCulling = nullptr;
	std::shared_ptr<MSceneGPUCulling> m_pGpuCulling = nullptr;

	std::unordered_map<const MType*, std::unique_ptr<IRenderWork>> m_tRenderWork;

	bool m_bGPUCullingUpdate = true;

	size_t m_nFrameIndex = 0;
};

#endif
