#include "MRenderSystem.h"


#include "MScene.h"
#include "MEngine.h"
#include "MVulkanDevice.h"
#include "MVulkanRenderCommand.h"

#include "MRenderableMeshComponent.h"


MORTY_CLASS_IMPLEMENT(MRenderSystem, MISystem)

MRenderSystem::MRenderSystem()
	: MISystem()
	, m_pDevice(nullptr)
{

}

MRenderSystem::~MRenderSystem()
{

}

void MRenderSystem::Update(MTaskNode* pNode)
{
	m_pDevice->Update();

}

MIDevice* MRenderSystem::GetDevice()
{
	return m_pDevice;
}

void MRenderSystem::OnTransformDirty(MComponent* pSender)
{
	if (!pSender)
		return;

	MEntity* pEntity = pSender->GetEntity();

	if (MRenderableMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderableMeshComponent>())
	{
		pMeshComponent->OnTransformDirty();
	}
}

void MRenderSystem::Initialize()
{
	if (!m_pDevice)
	{
		m_pDevice = new MVulkanDevice();
		m_pDevice->SetEngine(GetEngine());
		m_pDevice->Initialize();
	}

}

void MRenderSystem::Release()
{
	if (m_pDevice)
	{
		m_pDevice->Release();
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}

void MRenderSystem::ResizeFrameBuffer(MRenderPass& renderpass, const Vector2& v2Size)
{
	for (MTexture* pTexture : renderpass.m_vBackTextures)
	{
		pTexture->SetSize(v2Size);
		pTexture->DestroyBuffer(GetDevice());
		pTexture->GenerateBuffer(GetDevice());
	}

	if (renderpass.m_pDepthTexture)
	{
		renderpass.m_pDepthTexture->SetSize(v2Size);
		renderpass.m_pDepthTexture->DestroyBuffer(GetDevice());
		renderpass.m_pDepthTexture->GenerateBuffer(GetDevice());
	}

	renderpass.Resize(GetDevice());
}

void MRenderSystem::ReleaseRenderpass(MRenderPass& renderpass)
{
	for (MTexture* pTexture : renderpass.m_vBackTextures)
	{
		pTexture->DestroyBuffer(GetDevice());
		delete pTexture;
		pTexture = nullptr;
	}

	if (renderpass.m_pDepthTexture)
	{
		renderpass.m_pDepthTexture->DestroyBuffer(GetDevice());
		delete renderpass.m_pDepthTexture;
		renderpass.m_pDepthTexture = nullptr;
	}

	renderpass.DestroyBuffer(GetDevice());
}
