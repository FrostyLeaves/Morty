#include "MaterialPropertyRenderer.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "Resource/MMeshResourceUtil.h"
#include "Resource/MSkeletonResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"
#include "System/MSceneSystem.h"
#include "imgui.h"

using namespace morty;

MaterialPropertyRenderer::MaterialPropertyRenderer() {}

MaterialPropertyRenderer::~MaterialPropertyRenderer() {}

void MaterialPropertyRenderer::Initialize(MainEditor* pMainEditor)
{
    MEngine* pEngine = pMainEditor->GetEngine();

    auto*    sceneSystem    = pEngine->FindSystem<MSceneSystem>();
    auto*    objectSystem   = pEngine->FindSystem<MObjectSystem>();
    auto*    resourceSystem = pEngine->FindSystem<MResourceSystem>();

    m_scene = objectSystem->CreateObject<MScene>();

    m_sceneTexture = pMainEditor->CreateSceneViewer("MaterialPreview", m_scene);
    m_sceneTexture->SetRect(Vector2i(0, 0), Vector2i(512, 512));

    if (MEntity* pCameraNode = m_sceneTexture->GetViewport()->GetCamera())
    {
        if (auto* pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
        {
            pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
            pCameraSceneComponent->SetRotation(UnitQuaternion);
        }
    }

    m_staticSphereMeshNode = m_scene->CreateEntity();

    if (auto* meshComponent = m_staticSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
    {
        std::shared_ptr<MMeshResource> pMeshResource = resourceSystem->CreateResource<MMeshResource>();
        pMeshResource->Load(MMeshResourceUtil::CreateSphere());
        meshComponent->Load(pMeshResource);
    }

    sceneSystem->SetVisible(m_staticSphereMeshNode, false);

    m_skeletonSphereMeshNode = m_scene->CreateEntity();

    if (auto* pModelComponent = m_skeletonSphereMeshNode->RegisterComponent<MModelComponent>())
    {
        std::shared_ptr<MSkeletonResource> pSkeleton = resourceSystem->CreateResource<MSkeletonResource>();
        pModelComponent->SetSkeletonResource(pSkeleton);
    }

    if (auto* meshComponent = m_skeletonSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
    {
        std::shared_ptr<MMeshResource> pMeshResource = resourceSystem->CreateResource<MMeshResource>();
        pMeshResource->Load(MMeshResourceUtil::CreateSphere(MEMeshVertexType::Skeleton));
        meshComponent->Load(pMeshResource);
    }

    sceneSystem->SetVisible(m_skeletonSphereMeshNode, false);


    MEntity* pDirLight = m_scene->CreateEntity();
    pDirLight->SetName("DirLight");

    if (auto* pDirLightSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
    {
        Quaternion quat;
        quat.SetEulerAngle(Vector3(-45, 45, 0));
        pDirLightSceneComponent->SetRotation(Quaternion(quat));
    }

    if (auto* pDirLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
    {
        pDirLightComponent->SetLightIntensity(10.0f);
    }
}

void MaterialPropertyRenderer::Release(MainEditor* pMainEditor)
{
    SetMaterial(nullptr);

    m_scene->DeleteEntity(m_skeletonSphereMeshNode);
    m_skeletonSphereMeshNode = nullptr;
    m_scene->DeleteEntity(m_staticSphereMeshNode);
    m_staticSphereMeshNode = nullptr;
    m_scene->DeleteLater();
    m_scene = nullptr;

    pMainEditor->DestroySceneViewer(m_sceneTexture);
    m_sceneTexture = nullptr;
}

void MaterialPropertyRenderer::SetMaterial(std::shared_ptr<MMaterialResource> material)
{
    if (m_material == material) return;

    auto* sceneSystem = m_sceneTexture->GetViewport()->GetEngine()->FindSystem<MSceneSystem>();

    m_material = material;

    if (!m_material || !m_material->GetTemplate())
    {
        sceneSystem->SetVisible(m_staticSphereMeshNode, false);
        sceneSystem->SetVisible(m_skeletonSphereMeshNode, false);
    }
    else if (m_material->GetShaderMacro().GetMacro(MRenderGlobal::SHADER_SKELETON_ENABLE).empty())
    {
        sceneSystem->SetVisible(m_staticSphereMeshNode, true);
        sceneSystem->SetVisible(m_skeletonSphereMeshNode, false);

        if (auto* meshComponent = m_staticSphereMeshNode->GetComponent<MRenderMeshComponent>())
        {
            meshComponent->SetMaterial(material);
        }
    }
    else
    {
        sceneSystem->SetVisible(m_staticSphereMeshNode, false);
        sceneSystem->SetVisible(m_skeletonSphereMeshNode, true);

        if (auto* meshComponent = m_skeletonSphereMeshNode->GetComponent<MRenderMeshComponent>())
        {
            meshComponent->SetMaterial(material);
        }
    }
}

void MaterialPropertyRenderer::RenderMaterialProperties(std::shared_ptr<MMaterialResource> material)
{
    SetMaterial(material);

    if (m_material)
    {
        m_propertyBase.BindEngine(m_sceneTexture->GetViewport()->GetEngine());

        ImGui::Text("%s", m_material->GetResourcePath().c_str());
        ImGui::Separator();

        if (MTexturePtr texture = m_sceneTexture->GetFinalOutputTexture())
        {
            float fImageSize = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(fImageSize * 0.25f);
            ImGui::Image({texture, intptr_t(texture.get()), 0}, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        ImGui::Columns(2);
        ImGui::Separator();

        const bool bModify = m_propertyBase.EditMMaterial(m_material);
        m_sceneTexture->SetPauseUpdate(!bModify);


        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::PopStyleVar();
    }
    else { m_sceneTexture->SetPauseUpdate(true); }
}
