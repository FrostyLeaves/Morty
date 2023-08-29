#include "MEnvironmentManager.h"

#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Utility/MFunction.h"

#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MNotifyManager.h"
#include "System/MResourceSystem.h"
#include "Component/MSceneComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Resource/MMaterialResource.h"

MORTY_CLASS_IMPLEMENT(MEnvironmentManager, IManager)

void MEnvironmentManager::Initialize()
{
	Super::Initialize();
	InitializeMaterial();

	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_SKYBOX_TEX_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSkyBoxTextureChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_DIFFUSE_ENV_TEX_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnDiffuseEnvTextureChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_SPECULAR_ENV_TEX_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSpecularEnvTextureChanged, this));
	}
}

void MEnvironmentManager::Release()
{
	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_SKYBOX_TEX_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSkyBoxTextureChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_DIFFUSE_ENV_TEX_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnDiffuseEnvTextureChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_SPECULAR_ENV_TEX_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSpecularEnvTextureChanged, this));
	}

	ReleaseMaterial();
	Super::Release();
}

std::set<const MType*> MEnvironmentManager::RegisterComponentType() const
{
	return { MSkyBoxComponent::GetClassType() };
}

void MEnvironmentManager::RegisterComponent(MComponent* pComponent)
{
	MSkyBoxComponent* pSkyBoxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
	if (!pSkyBoxComponent)
	{
		return;
	}

	m_tAllSkyBoxComponent.insert(pSkyBoxComponent);

	if (m_pCurrentSkyBoxComponent == nullptr)
	{
		m_pCurrentSkyBoxComponent = pSkyBoxComponent;

		UpdateSkyBoxMaterial(m_pCurrentSkyBoxComponent);
	}

}

void MEnvironmentManager::UnregisterComponent(MComponent* pComponent)
{
	MSkyBoxComponent* pSkyBoxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
	if (!pSkyBoxComponent)
	{
		return;
	}

	m_tAllSkyBoxComponent.erase(pSkyBoxComponent);

	if (m_pCurrentSkyBoxComponent == pComponent)
	{
		m_pCurrentSkyBoxComponent = nullptr;
	}

	if (nullptr == m_pCurrentSkyBoxComponent && !m_tAllSkyBoxComponent.empty())
	{
		m_pCurrentSkyBoxComponent = *m_tAllSkyBoxComponent.begin();
	}
}

void MEnvironmentManager::OnSkyBoxTextureChanged(MComponent* pComponent)
{
	if (m_pCurrentSkyBoxComponent != pComponent)
	{
		return;
	}

	UpdateSkyBoxMaterial(m_pCurrentSkyBoxComponent);
}

void MEnvironmentManager::OnDiffuseEnvTextureChanged(MComponent* pComponent)
{
	if (m_pCurrentSkyBoxComponent != pComponent)
	{
		return;
	}

	auto pSkyboxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
	if (!pSkyboxComponent)
	{
		return;
	}

}

void MEnvironmentManager::OnSpecularEnvTextureChanged(MComponent* pComponent)
{
	if (m_pCurrentSkyBoxComponent != pComponent)
	{
		return;
	}

	auto pSkyboxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
	if (!pSkyboxComponent)
	{
		return;
	}


}

void MEnvironmentManager::UpdateSkyBoxMaterial(MSkyBoxComponent* pComponent)
{
	if (!m_pSkyBoxMaterial)
	{
		MORTY_ASSERT(m_pSkyBoxMaterial);
		return;
	}

	m_pSkyBoxMaterial->SetTexture("SkyTexCube", pComponent->GetSkyBoxResource());
}

bool MEnvironmentManager::HasEnvironmentComponent() const
{
	return m_pCurrentSkyBoxComponent != nullptr;
}

std::shared_ptr<MMaterial> MEnvironmentManager::GetMaterial() const
{
	return m_pSkyBoxMaterial;
}

void MEnvironmentManager::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> skyboxVS = pResourceSystem->LoadResource("Shader/Environment/skybox.mvs");
	std::shared_ptr<MResource> skyboxPS = pResourceSystem->LoadResource("Shader/Environment/skybox.mps");
	m_pSkyBoxMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pSkyBoxMaterial->SetCullMode(MECullMode::ECullNone);
	m_pSkyBoxMaterial->LoadVertexShader(skyboxVS);
	m_pSkyBoxMaterial->LoadPixelShader(skyboxPS);

}

void MEnvironmentManager::ReleaseMaterial()
{
	m_pSkyBoxMaterial = nullptr;
}
