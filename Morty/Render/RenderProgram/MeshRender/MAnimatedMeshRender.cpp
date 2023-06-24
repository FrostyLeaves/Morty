#include "MAnimatedMeshRender.h"

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
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"


MORTY_CLASS_IMPLEMENT(MAnimatedMeshRender, MObject)

MAnimatedMeshRender::MAnimatedMeshRender()
	: MObject()
{

}

MAnimatedMeshRender::~MAnimatedMeshRender()
{

}

void MAnimatedMeshRender::OnCreated()
{
	Super::OnCreated();
}

void MAnimatedMeshRender::OnDelete()
{

}

void MAnimatedMeshRender::Initialize(std::shared_ptr<MTexture> pTexture)
{
}

void MAnimatedMeshRender::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MAnimatedMeshRender::SetRenderPassAdapter(const std::shared_ptr<IRenderPassAdapter>& pAdapter)
{
	m_pRenderPassAdapter = pAdapter;
}
/*
void MAnimatedMeshRender::Render(MRenderInfo& info)
{

	if (!m_pFramePropertyAdapter)
	{
		MORTY_ASSERT(m_pFramePropertyAdapter);
		return;
	}

	if (!m_pRenderPassAdapter)
	{
		MORTY_ASSERT(m_pRenderPassAdapter);
		return;
	}

	MRenderPass* pRenderPass = m_pRenderPassAdapter->GetRenderPass();
	if (!pRenderPass)
	{
		MORTY_ASSERT(pRenderPass);
		return;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	MViewport* pViewport = info.pViewport;

	pCommand->BeginRenderPass(pRenderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	for (const auto& renderable : info.m_tDeferredMaterialGroupMesh)
	{
		DrawStaticMesh(pCommand, renderable.second);
	}

	pCommand->EndRenderPass();
}

void MAnimatedMeshRender::DrawAnimatedMesh(MIRenderCommand* pCommand, const MAnimatedMeshRenderable& renderable)
{
	if (!m_pFramePropertyAdapter)
	{
		MORTY_ASSERT(m_pFramePropertyAdapter);
		return;
	}

	const auto pFramePropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
	if (!pFramePropertyBlock)
	{
		MORTY_ASSERT(pFramePropertyBlock);
		return;
	}

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	const auto& pMaterial = renderable.pMaterial;
	const auto& vMeshComponent = renderable.vMeshComponents;

	pCommand->SetUseMaterial(pMaterial);
	pCommand->SetShaderPropertyBlock(pFramePropertyBlock);

	for (MRenderMeshComponent* pMeshComponent : vMeshComponent)
	{
		std::shared_ptr<MSkeletonInstance> pSkeletonIns = pMeshComponent->GetSkeletonInstance();
		if (!pSkeletonIns)
		{
			MORTY_ASSERT(pSkeletonIns);
			continue;
		}
		pCommand->SetShaderPropertyBlock(pSkeletonIns->GetShaderPropertyBlock());
		pCommand->SetShaderPropertyBlock(pMeshComponent->GetShaderMeshPropertyBlock());

		const MMeshManager::MMeshData& meshData = pMeshManager->FindMesh(pMeshComponent->GetMesh());

		pCommand->DrawMesh(
			pMeshManager->GetVertexBuffer(),
			pMeshManager->GetIndexBuffer(),
			meshData.vertexMemoryInfo.begin,
			meshData.indexMemoryInfo.begin,
			meshData.indexMemoryInfo.size);
	}
}
*/