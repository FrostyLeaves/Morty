#include "MEnvironmentManager.h"
#include "Component/MSceneComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MScene.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MFunction.h"
#include "Utility/MMaterialName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MEnvironmentManager, IManager)

void MEnvironmentManager::Initialize()
{
    Super::Initialize();
    InitializeMaterial();

    if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        pNotifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_SKYBOX_TEX_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSkyBoxTextureChanged, this)
        );
        pNotifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_DIFFUSE_ENV_TEX_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnDiffuseEnvTextureChanged, this)
        );
        pNotifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_SPECULAR_ENV_TEX_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSpecularEnvTextureChanged, this)
        );
    }
}

void MEnvironmentManager::Release()
{
    if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        pNotifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_SKYBOX_TEX_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSkyBoxTextureChanged, this)
        );
        pNotifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_DIFFUSE_ENV_TEX_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnDiffuseEnvTextureChanged, this)
        );
        pNotifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_SPECULAR_ENV_TEX_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MEnvironmentManager::OnSpecularEnvTextureChanged, this)
        );
    }

    ReleaseMaterial();
    Super::Release();
}

std::set<const MType*> MEnvironmentManager::RegisterComponentType() const { return {MSkyBoxComponent::GetClassType()}; }

void                   MEnvironmentManager::RegisterComponent(MComponent* pComponent)
{
    MSkyBoxComponent* pSkyBoxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
    if (!pSkyBoxComponent) { return; }

    m_allSkyBoxComponent.insert(pSkyBoxComponent);

    if (m_currentSkyBoxComponent == nullptr)
    {
        m_currentSkyBoxComponent = pSkyBoxComponent;

        UpdateSkyBoxMaterial(m_currentSkyBoxComponent);
    }
}

void MEnvironmentManager::UnregisterComponent(MComponent* pComponent)
{
    MSkyBoxComponent* pSkyBoxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
    if (!pSkyBoxComponent) { return; }

    m_allSkyBoxComponent.erase(pSkyBoxComponent);

    if (m_currentSkyBoxComponent == pComponent) { m_currentSkyBoxComponent = nullptr; }

    if (nullptr == m_currentSkyBoxComponent && !m_allSkyBoxComponent.empty())
    {
        m_currentSkyBoxComponent = *m_allSkyBoxComponent.begin();
    }
}

void MEnvironmentManager::OnSkyBoxTextureChanged(MComponent* pComponent)
{
    if (m_currentSkyBoxComponent != pComponent) { return; }

    UpdateSkyBoxMaterial(m_currentSkyBoxComponent);
}

void MEnvironmentManager::OnDiffuseEnvTextureChanged(MComponent* pComponent)
{
    if (m_currentSkyBoxComponent != pComponent) { return; }

    auto pSkyboxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
    if (!pSkyboxComponent) { return; }
}

void MEnvironmentManager::OnSpecularEnvTextureChanged(MComponent* pComponent)
{
    if (m_currentSkyBoxComponent != pComponent) { return; }

    auto pSkyboxComponent = pComponent->template DynamicCast<MSkyBoxComponent>();
    if (!pSkyboxComponent) { return; }
}

void MEnvironmentManager::UpdateSkyBoxMaterial(MSkyBoxComponent* pComponent)
{
    if (!m_skyBoxMaterial)
    {
        MORTY_ASSERT(m_skyBoxMaterial);
        return;
    }

    m_skyBoxMaterial->SetTexture(MShaderPropertyName::ENVIRONMENT_TEXTURE_SKYBOX, pComponent->GetSkyBoxResource());
}

bool MEnvironmentManager::HasEnvironmentComponent() const { return m_currentSkyBoxComponent != nullptr; }

std::shared_ptr<MMaterial> MEnvironmentManager::GetMaterial() const { return m_skyBoxMaterial; }

void                       MEnvironmentManager::InitializeMaterial()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    const auto       pTemplate = pResourceSystem->LoadResource(MMaterialName::SKY_BOX);
    m_skyBoxMaterial           = MMaterial::CreateMaterial(pTemplate);
}

void MEnvironmentManager::ReleaseMaterial() { m_skyBoxMaterial = nullptr; }
