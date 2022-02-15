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
	for (MBackTexture& tex : renderpass.m_vBackTextures)
	{
		tex.pTexture->SetSize(v2Size);
		tex.pTexture->DestroyBuffer(GetDevice());
		tex.pTexture->GenerateBuffer(GetDevice());
	}

	if (MTexture* pDepthTexture = renderpass.GetDepthTexture())
	{
		pDepthTexture->SetSize(v2Size);
		pDepthTexture->DestroyBuffer(GetDevice());
		pDepthTexture->GenerateBuffer(GetDevice());
	}

	renderpass.Resize(GetDevice());
}

void MRenderSystem::ReleaseRenderpass(MRenderPass& renderpass, bool bClearTexture)
{
	if (bClearTexture)
	{
		for (MBackTexture& tex : renderpass.m_vBackTextures)
		{
			tex.pTexture->DestroyBuffer(GetDevice());
			delete tex.pTexture;
			tex.pTexture = nullptr;
		}

		if (MTexture* pDepthTexture = renderpass.GetDepthTexture())
		{
			pDepthTexture->DestroyBuffer(GetDevice());
			delete pDepthTexture;
			pDepthTexture = nullptr;
		}
	}
	
	renderpass.m_vBackTextures.clear();
	renderpass.SetDepthTexture(nullptr, {});

	renderpass.DestroyBuffer(GetDevice());
}
