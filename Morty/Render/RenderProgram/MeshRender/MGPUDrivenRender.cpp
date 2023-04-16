#include "MGPUDrivenRender.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"

#include "MergeInstancing/MRenderableMeshGroup.h"

void MGPUDrivenRender::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MGPUDrivenRender::SetDrawIndirect(const std::shared_ptr<IDrawIndirectAdapter>& pDrawIndirectAdapter)
{
	m_pDrawIndirectAdapter = pDrawIndirectAdapter;
}

void MGPUDrivenRender::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MGPUDrivenRender::SetMeshInstancePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pMeshPropertyAdapter = pAdapter;
}

void MGPUDrivenRender::Render(MIRenderCommand* pCommand)
{
	if (!m_pScene)
	{
		MORTY_ASSERT(m_pScene);
		return;
	}

	if (!m_pFramePropertyAdapter)
	{
		MORTY_ASSERT(m_pFramePropertyAdapter);
		return;
	}

	
	const size_t nDrawIndirectNum = m_pDrawIndirectAdapter->GetCount();
	if (nDrawIndirectNum == 0)
	{
		return;
	}

	MScene* pScene = m_pScene;
	const MMeshManager* pMeshManager = pScene->GetEngine()->FindGlobalObject<MMeshManager>();
	const MBuffer* pDrawIndirectBuffer = m_pDrawIndirectAdapter->GetDrawIndirectBuffer();

	const auto pMaterial = m_pDrawIndirectAdapter->GetMaterial();
	pCommand->SetUseMaterial(pMaterial);
	
	auto vPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
	for (const auto& property : vPropertyBlock)
	{
		pCommand->SetShaderParamSet(property);
	}

	pCommand->DrawIndexedIndirect(
		pMeshManager->GetVertexBuffer(),
		pMeshManager->GetIndexBuffer(),
		pDrawIndirectBuffer,
		m_pDrawIndirectAdapter->GetOffset(),
		m_pDrawIndirectAdapter->GetCount()
	);
}

void MComputeDispatcherRender::SetComputeDispatcher(const std::shared_ptr<IComputeDispatcherAdapter>& pComputeDispatcher)
{
	m_pComputeDispatcher = pComputeDispatcher;
}

void MComputeDispatcherRender::Render(MIRenderCommand* pCommand)
{
	if (!m_pComputeDispatcher)
	{
		MORTY_ASSERT(m_pComputeDispatcher);
		return;
	}

	MComputeDispatcher* pComputeDispatcher = m_pComputeDispatcher->GetComputeDispatcher();
	if (!pComputeDispatcher)
	{
		MORTY_ASSERT(pComputeDispatcher);
		return;
	}

	const auto vComputeGroup = m_pComputeDispatcher->GetComputeGroup();
	auto vBarrierBuffer = m_pComputeDispatcher->GetBarrierBuffer();

	const size_t nGroupX = vComputeGroup[0];
	const size_t nGroupY = vComputeGroup[1];
	const size_t nGroupZ = vComputeGroup[2];

	pCommand->AddGraphToComputeBarrier({ vBarrierBuffer });
	pCommand->DispatchComputeJob(pComputeDispatcher, nGroupX, nGroupY, nGroupZ);
	pCommand->AddComputeToGraphBarrier({ vBarrierBuffer });
}
