#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MRenderModule.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "Scene/MScene.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MMaterialName.h"

using namespace morty;

void TRANSFORM_SPHERE_GENERATE(MEngine* pEngine, MScene* pScene)
{
    MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();


    const auto       pTemplate         = pResourceSystem->LoadResource(MMaterialName::DEFERRED_GBUFFER);
    const auto       pDeferredMaterial = MMaterialResource::CreateMaterial(pTemplate);
    {
        pDeferredMaterial->SetValue(MShaderPropertyName::MATERIAL_METALLIC, 1.0f);
        pDeferredMaterial->SetValue(MShaderPropertyName::MATERIAL_ROUGHNESS, 1.0f);
        pDeferredMaterial->SetValue(MShaderPropertyName::MATERIAL_ALBEDO, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

        std::shared_ptr<MResource> albedo    = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
        std::shared_ptr<MResource> normal    = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
        std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
        std::shared_ptr<MResource> ao        = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
        std::shared_ptr<MResource> metal     = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
        std::shared_ptr<MResource> height    = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

        pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO, albedo);
        pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);
        pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_METALLIC, metal);
        pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS, roughness);
        pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC, ao);
        pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT, height);
    }

    std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
    pCubeResource->Load(MMeshResourceUtil::CreateSphere());

    MEntity* pParent = pScene->CreateEntity();
    pParent->SetName("Parent");
    if (MSceneComponent* pSceneComponent = pParent->RegisterComponent<MSceneComponent>())
    {
        pSceneComponent->SetScale(Vector3(1.0f, 1.0f, 1.0f));
        pSceneComponent->SetRotation(Quaternion::FromEuler(Vector3(0.0f, 0.0f, 0.0f)));
    }

    if (MRenderMeshComponent* pMeshComponent = pParent->RegisterComponent<MRenderMeshComponent>())
    {
        pMeshComponent->SetMaterial(pDeferredMaterial);
        pMeshComponent->Load(pCubeResource);
    }


    MEntity* pChild = pScene->CreateEntity();
    pChild->SetName("Child");
    if (MSceneComponent* pSceneComponent = pChild->RegisterComponent<MSceneComponent>())
    {
        pSceneComponent->SetPosition(Vector3(5.0f, 0.0f, 0.0f));
        pSceneComponent->SetScale(Vector3(1.0f, 10.0f, 1.0f));
        pSceneComponent->SetRotation(Quaternion(Vector3(0, 0, 1), 45));
        pSceneComponent->SetParent(pParent->GetComponent<MSceneComponent>());
    }

    if (MRenderMeshComponent* pMeshComponent = pChild->RegisterComponent<MRenderMeshComponent>())
    {
        pMeshComponent->SetMaterial(pDeferredMaterial);
        pMeshComponent->Load(pCubeResource);
    }
}
