#include "MSinglePassRenderWork.h"

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

MORTY_INTERFACE_IMPLEMENT(ISinglePassRenderWork, MTypeClass)

void ISinglePassRenderWork::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

#if MORTY_DEBUG
	m_renderPass.m_strDebugName = GetTypeName();
#endif
}

void ISinglePassRenderWork::Release(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

std::vector<std::shared_ptr<MTexture>> ISinglePassRenderWork::GetBackTextures() const
{
	return m_renderPass.GetBackTextures();
}

std::shared_ptr<MTexture> ISinglePassRenderWork::GetDepthTexture() const
{
	return m_renderPass.GetDepthTexture();
}

std::shared_ptr<IGetTextureAdapter> ISinglePassRenderWork::CreateOutput() const
{
	auto pOutput = std::make_shared<MGetTextureAdapter>(m_renderPass.GetBackTexture(0));

	return pOutput;
}

void ISinglePassRenderWork::Resize(Vector2i size)
{
	if (m_renderPass.GetFrameBufferSize() != size)
	{
		MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
		pRenderSystem->ResizeFrameBuffer(m_renderPass, size);
	}
}

void ISinglePassRenderWork::SetRenderTarget(const MRenderTargetGroup& renderTarget)
{
	m_renderPass.SetRenderTarget(renderTarget);

	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
	m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());
}
